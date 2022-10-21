/*
 * crossref.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 * 
 * crossref.c is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * crossref.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <assert.h>
#include <inttypes.h>

#include "sci-modules.h"
#include "sci-log.h"
#include "sci-backend.h"
#include "types.h"
#include "scipaper.h"
#include "nxjson.h"
#include "utils.h"
#include "sci-conf.h"

/** Module name every module is required to have this*/
#define MODULE_NAME		"crossref"

/** Module information */
G_MODULE_EXPORT BackendInfo backend_info = {
	/** Name of the module */
	.name = MODULE_NAME,
};

#define CROSSREF_URL_DOMAIN  "https://api.crossref.org/"
#define CROSSREF_METHOD_WORKS "works"
#define CROSSREF_METHOD_JOURNALS "journals"
#define CROSSREF_SELECT "DOI,ISSN,abstract,author,publisher,reference,volume,title,issue,page,published"
#define CROSSREF_QUERY_ITEM_LIMIT 1000

struct CrPriv
{
	char* email;
	int rateLimit;
	int id;
	int timeout;
};

static GString* cf_create_url(struct CrPriv *priv, const char* method, GSList* queryList)
{
	GString* url = g_string_new(CROSSREF_URL_DOMAIN);
	g_string_append(url, method);

	if(priv->email)
		queryList = g_slist_prepend(queryList, pair_new("mailto", priv->email));

	GString* query = buildQuery(queryList);
	g_string_append(url, query->str);
	g_string_free(query, true);
	g_slist_free_full(queryList, (void(*)(void*))pair_free);

	return url;
}

static const nx_json* cf_get_message(const nx_json* const json, const char* const expectedType)
{
	if(!json)
		return NULL;
	if(!g_str_equal(nx_json_get(json, "status")->text_value, "ok"))
	{
		sci_module_log(LL_WARN, "returned invalid status");
		return NULL;
	}
	if(!g_str_equal(nx_json_get(json, "message-type")->text_value, expectedType))
	{
		sci_module_log(LL_WARN, "returned message of type %s instead of %s", nx_json_get(json, "message-type")->text_value, expectedType);
		return NULL;
	}
	const nx_json* message = nx_json_get(json, "message");
	if(message->type == NX_JSON_NULL)
	{
		sci_module_log(LL_WARN, "Message dosent contain document entry");
		return NULL;
	}
	return message;
}

static void cf_add_information_from_journal(DocumentMeta* meta, struct CrPriv* priv)
{
	if(!meta->issn || (meta->publisher && meta->journal))
		return;

	sci_module_log(LL_DEBUG, "adding journal info");

	GString* url = g_string_new(CROSSREF_URL_DOMAIN);
	g_string_append(url, CROSSREF_METHOD_JOURNALS);
	g_string_append_c(url, '/');
	g_string_append(url, meta->issn);

	GString* jsonText = wgetUrl(url->str, priv->timeout);
	if(!jsonText)
		return;

	const nx_json* json = nx_json_parse_utf8(jsonText->str);

	if(json)
	{
		const nx_json* messageNode = cf_get_message(json, "journal");
		if(messageNode)
		{
			if(!meta->publisher)
				meta->publisher = g_strdup(nx_json_get(messageNode, "publisher")->text_value);
			if(!meta->journal)
			meta->journal = g_strdup(nx_json_get(messageNode, "title")->text_value);
		}
		nx_json_free(json);
	}
	g_string_free(jsonText, true);
}

static DocumentMeta* cf_parse_work_json(const nx_json* json, const DocumentMeta* metaIn, struct CrPriv* priv)
{
	if(!json)
		return NULL;

	DocumentMeta* meta = metaIn ? document_meta_copy(metaIn) : document_meta_new();

	meta->compleatedLookup = true;
	meta->url    = g_strdup(nx_json_get(json, "URL")->text_value);
	const nx_json* authorArray = nx_json_get(json, "author");
	GString* authorString = g_string_new(NULL);
	for(size_t i = 0; i < authorArray->length; ++i)
	{
		const nx_json* author = nx_json_item(authorArray, i);
		const nx_json* givenName = nx_json_get(author, "given");
		const nx_json* familyName = nx_json_get(author, "family");
		if(givenName->type == NX_JSON_STRING)
		{
			g_string_append(authorString, givenName->text_value);
			if(familyName->type == NX_JSON_STRING)
				g_string_append_c(authorString, ' ');
		}

		if(familyName->type == NX_JSON_STRING)
			g_string_append(authorString, familyName->text_value);

		if(i < authorArray->length-1)
			g_string_append(authorString, ", ");
	}
	meta->author = g_strdup(authorString->str);
	g_string_free(authorString, true);

	const nx_json* publishedArray = nx_json_get(nx_json_get(json, "published"), "date-parts");
	if(publishedArray->type == NX_JSON_ARRAY && publishedArray->length > 0)
		meta->year = nx_json_item(publishedArray, 0)->int_value;

	const nx_json* journalNode = nx_json_get(json, "referance");
	if(journalNode != NX_JSON_NULL )
	{
		meta->journal = g_strdup(nx_json_get(journalNode, "journal-title")->text_value);

		if(!meta->year)
		{
			const char* yearStr = nx_json_get(journalNode, "year")->text_value;
			if(yearStr)
				meta->year = g_ascii_strtoull(yearStr, NULL, 10);
		}
	}
	meta->publisher = g_strdup(nx_json_get(json, "publisher")->text_value);
	meta->volume = g_strdup(nx_json_get(json, "volume")->text_value);

	const nx_json* titleArray = nx_json_get(json, "title");
	meta->title = g_strdup(nx_json_item(titleArray, 0)->text_value);
	meta->abstract = g_strdup(nx_json_get(json, "abstract")->text_value);

	if(!meta->doi)
		meta->doi = g_strdup(nx_json_get(json, "DOI")->text_value);

	const nx_json* issnArray = nx_json_get(json, "ISSN");
	if(issnArray->type == NX_JSON_ARRAY && issnArray->length > 0)
		meta->issn = g_strdup(nx_json_item(issnArray, 0)->text_value);

	cf_add_information_from_journal(meta, priv);

	return meta;
}

static DocumentMeta** cf_fill_from_doi(size_t* count, const DocumentMeta* meta, struct CrPriv* priv)
{
	GString* url = g_string_new(CROSSREF_URL_DOMAIN);
	g_string_append(url, CROSSREF_METHOD_WORKS);
	g_string_append_c(url, '/');
	g_string_append(url, meta->doi);

	sci_module_log(LL_DEBUG, "%s: grabbing %s", __func__, url->str);
	GString* jsonText = wgetUrl(url->str, priv->timeout);
	g_string_free(url, true);

	DocumentMeta** ret = NULL;
	DocumentMeta* filledMeta = NULL;

	if(jsonText)
	{
		const nx_json* json = nx_json_parse_utf8(jsonText->str);
		const nx_json* message = cf_get_message(json, "work");
		if(message)
			filledMeta = cf_parse_work_json(message, meta, priv);
		else
			sci_module_log(LL_WARN, "%s: got invalid entry without a message node", __func__);
		nx_json_free(json);
	}

	if(filledMeta)
	{
		filledMeta->backendId = priv->id;
		*count = 1;
		ret = g_malloc0(sizeof(*ret));
		ret[0] = filledMeta;
	}
	else
	{
		sci_module_log(LL_WARN, "%s: failed to parse message", __func__);
	}

	if(jsonText)
		g_string_free(jsonText, true);

	return ret;
}

static DocumentMeta** cf_fill_try_work_query(const DocumentMeta* meta, size_t* count, size_t maxCount, struct CrPriv* priv)
{
	GSList* queryList = NULL;

	if(meta->author)
		queryList = g_slist_prepend(queryList, pair_new("query.author", meta->author));
	if(meta->title)
		queryList = g_slist_prepend(queryList, pair_new("query.title", meta->title));
	if(meta->journal)
		queryList = g_slist_prepend(queryList, pair_new("query.publisher-name", meta->journal));
	if(meta->hasFullText)
		queryList = g_slist_prepend(queryList, pair_new("filter", "has-full-text:true"));
	if(meta->year)
	{
		char* yearStr = g_strdup_printf("%lu", meta->year);
		queryList = g_slist_prepend(queryList, pair_new("query.bibliographic", yearStr));
		g_free(yearStr);
	}

	if(!queryList)
		return NULL;

	char* intStr = g_strdup_printf("%zu", maxCount);
	queryList = g_slist_prepend(queryList, pair_new("rows", intStr));
	g_free(intStr);
	queryList = g_slist_prepend(queryList, pair_new("select", CROSSREF_SELECT));

	DocumentMeta** documents = NULL;

	GString* url = cf_create_url(priv, CROSSREF_METHOD_WORKS, queryList);
	sci_module_log(LL_DEBUG, "%s: %s", __func__, url->str);
	GString* jsonText = wgetUrl(url->str, priv->timeout);
	if(jsonText)
	{
		sci_module_log(LL_DEBUG, "got text");
		const nx_json* json = nx_json_parse_utf8(jsonText->str);
		const nx_json* messageNode = cf_get_message(json, "work-list");
		if(messageNode)
		{
			long long int totalResults = nx_json_get(messageNode, "total-results")->int_value;
				sci_module_log(LL_DEBUG, "%s: got %lli results of which %zu will be processed",
				__func__, totalResults, (totalResults > maxCount ? maxCount : (size_t)totalResults));

			const nx_json* arrayNode = nx_json_get(messageNode, "items");
			if(arrayNode->type == NX_JSON_ARRAY)
			{
				size_t resultCount = arrayNode->length < maxCount ? arrayNode->length : maxCount;
				documents = g_malloc0(sizeof(*documents)*resultCount);
				for(size_t i = 0; i < resultCount; ++i)
				{
					const nx_json* item = nx_json_item(arrayNode, i);
					if(item->type != NX_JSON_NULL)
					{
						documents[i] = cf_parse_work_json(item, NULL, priv);
						documents[i]->backendId = priv->id;
					}
					else
					{
						documents[i] = NULL;
						sci_module_log(LL_WARN, "%s: invalid array item", __func__);
					}
				}
				*count = resultCount;
			}
			else
			{
				sci_module_log(LL_WARN, "%s: No items array node in work list", __func__);
			}
		}
		g_string_free(jsonText, true);
		nx_json_free(json);
	}
	g_string_free(url, true);
	return documents;
}

static DocumentMeta** cf_fill_meta_in(const DocumentMeta* meta, size_t* count, size_t maxCount, void* userData)
{
	struct CrPriv* priv = userData;
	if(maxCount == 0)
		return NULL;

	if(meta->doi)
		return cf_fill_from_doi(count, meta, priv);

	return cf_fill_try_work_query(meta, count, maxCount, priv);
}

G_MODULE_EXPORT const gchar *sci_module_init(void** data);
const gchar *sci_module_init(void** data)
{
	struct CrPriv* priv = g_malloc0(sizeof(*priv));
	priv->id = sci_plugin_register(&backend_info, cf_fill_meta_in, NULL, NULL, priv);
	priv->rateLimit = sci_conf_get_int("Crossref", "RateLimit", 10, NULL);
	priv->email = sci_conf_get_string("Crossref", "Email", NULL, NULL);
	priv->timeout = sci_conf_get_int("Crossref", "Timeout", 20, NULL);
	*data = priv;
	return NULL;
}

G_MODULE_EXPORT void sci_module_exit(void* data);
void sci_module_exit(void* data)
{
	struct CrPriv* priv = data;
	sci_plugin_unregister(priv->id);
	g_free(priv->email);
	g_free(priv);
}
