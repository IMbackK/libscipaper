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
#include <string.h>
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
	int retry;
	DocumentMeta* lastDocument;
	char* scrollId;
	int nextPage;
	int lastMaxCount;
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

static char* core_get_document_id(const nx_json* idArray)
{
	if(idArray->type != NX_JSON_ARRAY)
		return NULL;

	for(size_t i = 0; i < idArray->length; ++i)
	{
		const nx_json* identifier = nx_json_item(idArray, i);
		if(g_str_equal(nx_json_get(identifier, "type")->text_value, "CORE_ID"))
			return g_strdup(nx_json_get(identifier, "identifier")->text_value);
	}

	return NULL;
}

static char* core_get_document_doi(const nx_json* idArray)
{
	if(idArray->type != NX_JSON_ARRAY)
		return NULL;

	for(size_t i = 0; i < idArray->length; ++i)
	{
		const nx_json* identifier = nx_json_item(idArray, i);
		if(g_str_equal(nx_json_get(identifier, "type")->text_value, "DOI"))
			return g_strdup(nx_json_get(identifier, "identifier")->text_value);
	}

	return NULL;
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
	const struct nx_json* doiJson = nx_json_get(item, "doi");
	if(doiJson->type == NX_JSON_STRING && strlen(doiJson->text_value) > 5)
		result->doi = g_strdup(doiJson->text_value);
	else
		result->doi = core_get_document_doi(nx_json_get(item, "identifiers"));
	result->title = g_strdup(nx_json_get(item, "title")->text_value);
	result->publisher = g_strdup(nx_json_get(item, "publisher")->text_value);
	result->year = nx_json_get(item, "yearPublished")->int_value;
	result->downloadUrl = g_strdup(nx_json_get(item, "downloadUrl")->text_value);

	return result;
}

static bool core_is_in_range(int page, int nextPage)
{
	if(page < nextPage)
		return false;
	if(page - nextPage < 3)
		return true;
	return false;
}

static RequestReturn* core_fill_meta_impl(int *code, const DocumentMeta* meta, size_t maxCount,
										  size_t page, struct CorePriv* priv)
{
	RequestReturn* results = NULL;
	bool fastPage = false;
	if(page == 0 || (document_meta_is_equal(meta, priv->lastDocument) &&
		priv->lastMaxCount == maxCount && core_is_in_range(page, priv->nextPage) && priv->scrollId))
	{
		if(page != 0)
			sci_module_log(LL_DEBUG, "Using fast pageing for this request");
		fastPage = true;
	}
	else if(page != 0)
	{
		sci_module_log(LL_DEBUG, "Using slow pageing for this request %s page: %i expected: %i %s %s",
					document_meta_is_equal(meta, priv->lastDocument) ? "" : "metas are not equal",
					page, priv->nextPage, priv->scrollId ? "" : "no scrollId stored",
					priv->lastMaxCount == maxCount ? "" : "maxCounts are not equal");
	}

	char* intStr = g_strdup_printf("%zu", maxCount);
	GSList* queryList = g_slist_prepend(NULL, pair_new("limit", intStr));
	g_free(intStr);
	if(fastPage)
	{
		queryList = g_slist_prepend(queryList, pair_new("scroll", "true"));
		if(page > 0 && priv->scrollId)
			queryList = g_slist_prepend(queryList, pair_new("scrollId", priv->scrollId));
	}
	else
	{
		char* scrollStr = g_strdup_printf("%zu", page*maxCount);
		queryList = g_slist_prepend(queryList, pair_new("offset", scrollStr));
		g_free(scrollStr);
	}
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
		char** tokens = g_str_tokenize_and_fold(meta->keywords, NULL, NULL);
		for(size_t i = 0; tokens[i]; ++i)
		{
			g_string_append(searchString, tokens[i]);
			g_string_append_c(searchString, '+');
			g_free(tokens[i]);
		}
		g_free(tokens);
	}
	if(meta->abstract)
	{
		g_string_append(searchString, "abstract:\"");
		g_string_append(searchString, meta->abstract);
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
	GString* jsonText = wgetUrl(url->str, priv->timeout + maxCount);
	g_string_free(url, true);
	if(!jsonText)
	{
		*code = 1;
		return results;
	}

	const nx_json* json = nx_json_parse_utf8(jsonText->str);
	const nx_json* resutlsArray = nx_json_get(json, "results");
	if(resutlsArray->type != NX_JSON_ARRAY)
	{
		sci_module_log(LL_WARN, "%s: invalid response no results entry", __func__);
		nx_json_free(json);
		g_string_free(jsonText, true);
		*code = 2;
		return results;
	}

	results = request_return_new((size_t)resutlsArray->length, maxCount);
	if(!fastPage)
		results->page = (size_t)(nx_json_get(json, "offset")->int_value/maxCount);
	else
		results->page = page;
	results->totalCount = (size_t)nx_json_get(json, "totalHits")->int_value;

	for(size_t i = 0; i < results->count; ++i)
	{
		const nx_json* item = nx_json_item(resutlsArray, i);
		results->documents[i] = core_parse_document_meta(item, priv);
	}

	if(fastPage)
	{
		sci_module_log(LL_DEBUG, "Saveing scrollId for next request");
		priv->lastMaxCount = maxCount;
		priv->nextPage = ++page;
		document_meta_free(priv->lastDocument);
		priv->lastDocument = document_meta_copy(meta);
		g_free(priv->scrollId);
		priv->scrollId = g_strdup(nx_json_get(json, "scrollId")->text_value);
	}
	else
	{
		document_meta_free(priv->lastDocument);
		priv->lastDocument = NULL;
		g_free(priv->scrollId);
		priv->scrollId = NULL;
	}

	nx_json_free(json);
	g_string_free(jsonText, true);

	*code = 0;
	return results;
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

	if(meta->author || meta->title || meta->keywords || meta->searchText || meta->abstract)
	{
		int code = -1;
		for(int i = 0; i < priv->retry && code != 0; ++i)
		{
			if(i != 0)
				sci_module_log(LL_WARN, "Could not get results from core, retrying %i of %i", i+1, priv->retry);
			results = core_fill_meta_impl(&code, meta, maxCount, page, priv);
		}
	}
	else
	{
		sci_module_log(LL_DEBUG, "Can not fill meta that dose not contain author, title, keywords, abstract or searchText");
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

static char* core_get_arxiv_pdf_url(const char* arxivUrl)
{
	char* url = g_strdup(arxivUrl);
	char* absBeg = g_strstr_len(url, -1, "abs");
	if(!absBeg)
	{
		g_free(url);
		return NULL;
	}

	absBeg[0] = 'p';
	absBeg[1] = 'd';
	absBeg[2] = 'f';

	char* urlWithExtension = g_strconcat(url, ".pdf", NULL);
	g_free(url);

	return urlWithExtension;
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
	char* url;
	bool freeUrl = false;
	if(g_strstr_len(pdfMeta->downloadUrl, -1, "arxiv.org") != NULL)
	{
		url = core_get_arxiv_pdf_url(pdfMeta->downloadUrl);
		if(!url)
		{
			sci_module_log(LL_DEBUG, "Url is from arxiv, but unable to find real pdf url");
			return NULL;
		}
		sci_module_log(LL_DEBUG, "Url is from arxiv, diverting to %s", url);
		freeUrl = true;
	}
	else
	{
		url = pdfMeta->downloadUrl;
	}

	PdfData* pdfData = wgetPdf(url, priv->timeout);
	if(freeUrl)
		g_free(url);

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
	priv->retry = sci_conf_get_int("Core", "Retry", 1, NULL);
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
	g_free(priv->scrollId);
	document_meta_free(priv->lastDocument);
	g_free(priv);
}
