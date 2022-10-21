#include "sci-backend.h"
#include "scipaper.h"

#include <glib.h>
#include <stdbool.h>

#include "sci-log.h"

struct SciBackend
{
	DocumentMeta** (*fill_meta)(const DocumentMeta* meta, size_t* count, size_t maxCount, void* user_data);
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
	const BackendInfo* backend = sci_get_backend_info(id);
	if(!backend)
		return NULL;
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
						DocumentMeta** (*fill_meta_in)(const DocumentMeta* meta, size_t* count, size_t maxCount, void* user_data),
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

DocumentMeta** sci_fill_meta(const DocumentMeta* meta, size_t* count, size_t maxCount)
{
	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->fill_meta && (meta->backendId == backend->id || meta->backendId == 0))
		{
			DocumentMeta** newMeta = backend->fill_meta(meta, count, maxCount, backend->user_data);
			if(newMeta)
			{
				for(size_t i = 0; i < *count; ++i)
					newMeta[i]->compleatedLookup = true;
				return newMeta;
			}
		}
	}
	sci_log(LL_WARN, "Unable to fill meta %s", __func__);
	count = 0;
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
	sci_log(LL_WARN, "Unable to get text %s", __func__);
	return NULL;
}

PdfData* sci_get_document_pdf_data(const DocumentMeta* meta)
{
	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->get_document_pdf_data && (meta->backendId == backend->id || meta->backendId == 0))
		{
			PdfData* data = backend->get_document_pdf_data(meta, backend->user_data);
			if(data)
				return data;
		}
	}
	sci_log(LL_WARN, "Unable to get pdf data %s", __func__);
	return NULL;
}
