/*
 * sci-backend.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 *
 * sci-backend.c is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * sci-backend.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sci-backend.h"
#include "scipaper.h"

#include <glib.h>
#include <stdbool.h>

#include "sci-log.h"

struct SciBackend
{
	RequestReturn* (*fill_meta)(const DocumentMeta* meta, size_t page, size_t maxCount, void* user_data);
	char* (*get_document_text)(const DocumentMeta* meta, void* user_data);
	PdfData* (*get_document_pdf_data)(const DocumentMeta* meta, void* user_data);
	int id;
	const BackendInfo* backend_info;
	void* user_data;
};

static GSList *backends;
static const BackendInfo** backendsArray;

const BackendInfo** sci_get_all_backends(void)
{
	if(!backendsArray)
	{
		backendsArray = g_malloc(sizeof(*backendsArray)*(g_slist_length(backends)+1));
		size_t i = 0;
		for(GSList* element = backends; element; element = element->next)
		{
			struct SciBackend* backend = (struct SciBackend*)element->data;
			backendsArray[i] = backend->backend_info;
			++i;
		}

		backendsArray[i] = NULL;
	}
	return backendsArray;
}

const BackendInfo* sci_get_backend_info(int id)
{
	for(GSList* element = backends; element; element = element->next)
	{
		struct SciBackend* backend = (struct SciBackend*)element->data;
		if(backend->id == id)
		{
			return backend->backend_info;
		}
	}
	return NULL;
}

const char* sci_get_backend_name(int id)
{
	if(id == 0)
		return "Unkown/Any";
	const BackendInfo* backend = sci_get_backend_info(id);
	if(!backend)
		return "Invalid";
	return backend->name;
}

int sci_backend_get_id_by_name(const char* name)
{
	for(GSList* element = backends; element; element = element->next)
	{
		struct SciBackend* backend = (struct SciBackend*)element->data;
		if(g_str_equal(backend->backend_info->name, name))
		{
			return backend->id;
		}
	}
	return 0;
}

size_t sci_get_backend_count(void)
{
	return g_slist_length(backends);
}

int sci_plugin_register(const BackendInfo* backend_info,
						RequestReturn* (*fill_meta_in)(const DocumentMeta* meta, size_t page, size_t maxCount, void* user_data),
						char* (*get_document_text_in)(const DocumentMeta* meta, void* user_data),
						PdfData* (*get_document_pdf_data_in)(const DocumentMeta* meta, void* user_data), void* user_data)
{
	static int id_counter = 0;

	struct SciBackend* backend = g_malloc0(sizeof(*backend));

	backend->id = ++id_counter;
	backend->fill_meta = fill_meta_in;
	backend->get_document_text = get_document_text_in;
	backend->get_document_pdf_data = get_document_pdf_data_in;
	backend->backend_info = backend_info;
	backend->user_data = user_data;

	backends = g_slist_prepend(backends, backend);

	if(backendsArray)
	{
		g_free(backendsArray);
		backendsArray = NULL;
	}
	return id_counter;
}

void sci_plugin_unregister(int id)
{
	GSList *element;
	for(element = backends; element; element = element->next)
	{
		struct SciBackend* last_backend = (struct SciBackend*)element->data;
		if(last_backend->id == id)
			break;
	}
	if (!element)
		sci_log(LL_WARN, "Trying to remove non-existing comm backend with id %d", id);

	g_free(element->data);

	backends = g_slist_remove(backends, element->data);

	if(backendsArray)
	{
		g_free(backendsArray);
		backendsArray = NULL;
	}
}

static bool is_filled_as_requested(const DocumentMeta* meta, const FillReqest* fill)
{
	bool ret = true;

	if(!fill)
		return ret;

	if(fill->doi && !meta->doi)
		ret = false;
	if(fill->url && !meta->url)
		ret = false;
	if(fill->year && !meta->year)
		ret = false;
	if(fill->publisher && !meta->publisher)
		ret = false;
	if(fill->volume && !meta->volume)
		ret = false;
	if(fill->pages && !meta->pages)
		ret = false;
	if(fill->author && !meta->author)
		ret = false;
	if(fill->title && !meta->title)
		ret = false;
	if(fill->journal && !meta->journal)
		ret = false;
	if(fill->issn && !meta->issn)
		ret = false;
	if(fill->keywords && !meta->keywords)
		ret = false;
	if(fill->downloadUrl && !meta->downloadUrl)
		ret = false;
	if(fill->abstract && !meta->abstract)
		ret = false;
	return ret;
}

static void sci_compleat_fill_meta(DocumentMeta* meta, const FillReqest* fill)
{
	if(!meta->doi)
		return;

	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->id == meta->backendId)
			continue;
		sci_log(LL_DEBUG, "try filling with %s", sci_get_backend_name(backend->id));
		DocumentMeta* soruceMeta = sci_find_by_doi(meta->doi, backend->id);
		document_meta_combine(meta, soruceMeta);
		if(is_filled_as_requested(meta, fill))
			break;
	}
}

RequestReturn* sci_fill_meta(const DocumentMeta* meta, const FillReqest* fill, size_t maxCount, size_t page)
{
	if(meta->backendId != 0 && fill)
	{
		sci_log(LL_WARN,
				"%s: a search request with explicitly set backend id %i also has a FillReqest, it will be ignored\n",
				__func__, meta->backendId);
	}

	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->fill_meta && (meta->backendId == backend->id || meta->backendId == 0))
		{
			RequestReturn* newMetas = backend->fill_meta(meta, maxCount, page, backend->user_data);
			if(newMetas)
			{
				for(size_t i = 0; i < newMetas->count; ++i)
				{
					document_meta_combine(newMetas->documents[i], meta);
					if(meta->backendId == 0 && !is_filled_as_requested(newMetas->documents[i], fill))
					{
						sci_log(LL_DEBUG,
							"%s: Document found by %s but uncompeat filling:", __func__, sci_get_backend_name(backend->id));
						sci_compleat_fill_meta(newMetas->documents[i], fill);
					}
					newMetas->documents[i]->compleatedLookup = true;
				}
				return newMetas;
			}
		}
	}
	if(meta->backendId == 0)
		sci_log(LL_WARN, "%s: Unable to fill meta", __func__);
	else
		sci_log(LL_WARN, "%s: Unable to get meta from %s, maybe try without specifying a backend",
				__func__, sci_get_backend_name(meta->backendId));
	return NULL;
}

char* sci_get_document_text(const DocumentMeta* meta)
{
	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->get_document_text && (meta->backendId == backend->id || meta->backendId == 0))
		{
			char* text = backend->get_document_text(meta, backend->user_data);
			if(text)
				return text;
		}
	}
	if(meta->backendId == 0)
		sci_log(LL_WARN, "%s: Unable to get text", __func__);
	else
		sci_log(LL_WARN, "%s: Unable to get text from %s, maybe try without specifying a backend",
				__func__, sci_get_backend_name(meta->backendId));
	return NULL;
}

PdfData* sci_get_document_pdf_data(const DocumentMeta* meta)
{
	bool backendAvail = false;
	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->get_document_pdf_data && (meta->backendId == backend->id || meta->backendId == 0))
		{
			PdfData* data = backend->get_document_pdf_data(meta, backend->user_data);
			backendAvail = true;
			if(data)
				return data;
		}
	}
	if(meta->backendId == 0)
		sci_log(LL_WARN, "%s: Unable to get pdf data%s", __func__, backendAvail ? "" : " no backend available");
	else
		sci_log(LL_WARN, "%s: Unable to get pdf data from %s, maybe try without specifying a backend",
				__func__, sci_get_backend_name(meta->backendId));
	return NULL;
}
