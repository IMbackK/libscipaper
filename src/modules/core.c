/*
 * core.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 * 
 * core.c is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * core.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include "sci-modules.h"
#include "sci-log.h"
#include "sci-backend.h"
#include "types.h"
#include "scipaper.h"
#include "sci-conf.h"
#include "utils.h"
#include "nxjson.h"

/** Module name every module is required to have this*/
#define MODULE_NAME		"core"

/** Module information */
static BackendInfo backend_info = {
	/** Name of the module */
	.name = MODULE_NAME,
	.capabilities = SCI_CAP_FILL | SCI_CAP_GET_TEXT | SCI_CAP_GET_PDF
};

#define CORE_API_BASE_URL "https://api.core.ac.uk/v3/"
#define CORE_METHOD_SEARCH_WORKS "search/works/"
#define CORE_METHOD_OUTPUTS "outputs/"

struct CorePriv
{
	char* apiKey;
	int rateLimit;
	int id;
	int timeout;
};

struct CoreData
{
	char* fullText;
	char* id;
};

static void core_free_data(void* data)
{
	struct CoreData* coredata = data;
	g_free(coredata->fullText);
	g_free(coredata->id);
	g_free(coredata);
}

static void* core_copy_data(void* data)
{
	struct CoreData* coreData = data;
	struct CoreData* copy = g_malloc0(sizeof(*copy));
	copy->fullText = g_strdup(coreData->fullText);
	copy->id =  g_strdup(coreData->id);
	return copy;
}

static GString* core_create_url(struct CorePriv *priv, const char* method, GSList* queryList)
{
	GString* url = g_string_new(CORE_API_BASE_URL);
	g_string_append(url, method);
	queryList = g_slist_prepend(queryList, pair_new("apiKey", priv->apiKey));

	GString* query = buildQuery(queryList);
	g_string_append(url, query->str);
	g_string_free(query, true);
	g_slist_free_full(queryList, (void(*)(void*))pair_free);

	return url;
}

char* core_get_document_id(const nx_json* idArray)
{
	if(idArray->type != NX_JSON_ARRAY)
		return 0;

	for(size_t i = 0; i < idArray->length; ++i)
	{
		const nx_json* identifier = nx_json_item(idArray, i);
		if(g_str_equal(nx_json_get(identifier, "type")->text_value, "CORE_ID"))
			return g_strdup(nx_json_get(identifier, "identifier")->text_value);
	}

	return 0;
}

static DocumentMeta* core_parse_document_meta(const nx_json* item, struct CorePriv* priv)
{
	DocumentMeta *result = document_meta_new();
	result->backendId = priv->id;
	result->hasFullText = true;

	struct CoreData* coreData = g_malloc0(sizeof(*coreData));
	coreData->fullText = g_strdup(nx_json_get(item, "fullText")->text_value);
	coreData->id = core_get_document_id(nx_json_get(item, "identifiers"));
	coreData->fullText = g_strdup(nx_json_get(item, "fullText")->text_value);
	result->backendData = coreData;
	result->backend_data_free_fn = &core_free_data;
	result->backend_data_copy_fn = &core_copy_data;

	const nx_json* authorArray = nx_json_get(item, "authors");
	GString* authorString = g_string_new(NULL);
	for(size_t i = 0; i < authorArray->length; ++i)
	{
		const nx_json* author = nx_json_item(authorArray, i);
		const nx_json* name = nx_json_get(author, "name");
		if(name->type == NX_JSON_STRING)
			g_string_append(authorString, name->text_value);
		if(i < authorArray->length-1)
			g_string_append(authorString, ", ");
	}
	result->author = authorString->str;
	g_string_free(authorString, false);

	result->abstract = g_strdup(nx_json_get(item, "abstract")->text_value);
	result->doi = g_strdup(nx_json_get(item, "doi")->text_value);
	result->title = g_strdup(nx_json_get(item, "title")->text_value);
	result->publisher = g_strdup(nx_json_get(item, "publisher")->text_value);
	result->year = nx_json_get(item, "yearPublished")->int_value;
	result->downloadUrl = g_strdup(nx_json_get(item, "downloadUrl")->text_value);

	return result;
}

static RequestReturn* core_fill_meta(const DocumentMeta* meta, size_t maxCount, size_t page, void* userData)
{
	struct CorePriv* priv = userData;
	RequestReturn* results = NULL;

	if(maxCount == 0)
	{
		sci_module_log(LL_WARN, "A request for 0 results was given");
		return NULL;
	}

	if(meta->author || meta->title || meta->keywords || meta->searchText)
	{
		char* intStr = g_strdup_printf("%zu", maxCount);
		GSList* queryList = g_slist_prepend(NULL, pair_new("limit", intStr));
		g_free(intStr);
		queryList = g_slist_prepend(queryList, pair_new("scroll", "true"));
		char* scrollStr = g_strdup_printf("%zu", page*maxCount);
		queryList = g_slist_prepend(queryList, pair_new("offset", scrollStr));
		g_free(scrollStr);
		queryList = g_slist_prepend(queryList, pair_new("stats", "false"));

		GString* searchString = g_string_new(NULL);
		if(meta->author)
		{
			g_string_append(searchString, "authors:\"");
			g_string_append(searchString, meta->author);
			g_string_append(searchString, "\"+");
		}
		if(meta->title)
		{
			g_string_append(searchString, "title:\"");
			g_string_append(searchString, meta->title);
			g_string_append(searchString, "\"+");
		}
		if(meta->keywords)
		{
			char** tokens = g_str_tokenize_and_fold(meta->title, NULL, NULL);
			g_string_append(searchString, "title:\"");
			for(size_t i = 0; tokens[i]; ++i)
			{
				g_string_append(searchString, tokens[i]);
				g_free(tokens[i]);
			}
			g_free(tokens);
			g_string_append(searchString, meta->title);
			g_string_append(searchString, "\"+");

		}
		if(meta->searchText)
		{
			g_string_append_c(searchString, '\"');
			g_string_append(searchString, meta->searchText);
			g_string_append(searchString, "\"+");
		}
		g_string_truncate(searchString, searchString->len-1);
		queryList = g_slist_prepend(queryList, pair_new("q", searchString->str));
		g_string_free(searchString, true);

		GString* url = core_create_url(priv, CORE_METHOD_SEARCH_WORKS, queryList);
		sci_module_log(LL_DEBUG, "%s: getting url string: %s", __func__, url->str);
		GString* jsonText = wgetUrl(url->str, priv->timeout);
		g_string_free(url, true);
		if(!jsonText)
			return results;

		const nx_json* json = nx_json_parse_utf8(jsonText->str);
		const nx_json* resutlsArray = nx_json_get(json, "results");
		if(resutlsArray->type != NX_JSON_ARRAY)
		{
			sci_module_log(LL_WARN, "%s: invalid response no results entry", __func__);
			nx_json_free(json);
			g_string_free(jsonText, true);
			return results;
		}

		results = request_return_new((size_t)resutlsArray->length, maxCount);
		results->page = (size_t)(nx_json_get(json, "offset")->int_value/maxCount);
		results->totalCount = (size_t)nx_json_get(json, "totalHits")->int_value;

		for(size_t i = 0; i < results->count; ++i)
		{
			const nx_json* item = nx_json_item(resutlsArray, i);
			results->documents[i] = core_parse_document_meta(item, priv);
		}

		nx_json_free(json);
		g_string_free(jsonText, true);
	}

	return results;
}

char* core_get_document_text(const DocumentMeta* meta, void* userData)
{
	struct CorePriv* priv = userData;

	if(meta->backendId == priv->id && meta->backendData)
	{
		struct CoreData* data = meta->backendData;
		return g_strdup(data->fullText);
	}
	else
	{
		RequestReturn* metas = core_fill_meta(meta, 1, 0, priv);
		if(!metas)
		{
			return NULL;
		}
		else
		{
			char* text = g_strdup(((struct CoreData*)metas->documents[0]->backendData)->fullText);
			request_return_free(metas);
			return text;
		}
	}

	return NULL;
}

static PdfData* core_get_document_pdf_data(const DocumentMeta* meta, void* userData)
{
	sci_module_log(LL_DEBUG, "%s got meta from %i", __func__, meta->backendId);
	struct CorePriv* priv = userData;
	DocumentMeta* pdfMeta = NULL;

	if(meta->backendId == priv->id)
	{
		pdfMeta = document_meta_copy(meta);
	}
	else
	{
		if(meta->doi)
			pdfMeta = sci_find_by_doi(meta->doi, priv->id);
		if(!pdfMeta)
		{
			sci_module_log(LL_DEBUG, "unable to fill for doi %s to get pdf", meta->doi);
			return NULL;
		}
	}

	if(!pdfMeta->downloadUrl)
		return NULL;

	sci_module_log(LL_DEBUG, "Trying to get pdf from %s", pdfMeta->downloadUrl);
	PdfData* pdfData = wgetPdf(pdfMeta->downloadUrl, priv->timeout);

	if(pdfData)
		pdfData->meta = pdfMeta;
	else
		document_meta_free(pdfMeta);

	return pdfData;
}

G_MODULE_EXPORT const gchar *sci_module_init(void** data);
const gchar *sci_module_init(void** data)
{
	struct CorePriv* priv = g_malloc0(sizeof(*priv));

	priv->rateLimit = sci_conf_get_int("Core", "RateLimit", 10, NULL);
	priv->apiKey = sci_conf_get_string("Core", "ApiKey", NULL, NULL);
	priv->timeout = sci_conf_get_int("Core", "Timeout", 20, NULL);

	*data = priv;

	if(!priv->apiKey)
		return "This module can not work without an api key, you must set this key in Core/ApiKey in the config file";

	priv->id = sci_plugin_register(&backend_info, core_fill_meta, core_get_document_text, core_get_document_pdf_data, priv);

	return NULL;
}

G_MODULE_EXPORT void sci_module_exit(void* data);
void sci_module_exit(void* data)
{
	struct CorePriv* priv = data;
	sci_plugin_unregister(priv->id);
	g_free(priv->apiKey);
	g_free(priv);
}
