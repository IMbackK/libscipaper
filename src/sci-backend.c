#include "sci-backend.h"

#include <glib.h>
#include <stdbool.h>

#include "sci-log.h"

struct SciBackend
{
	DocumentMeta** (*fill_meta)(const DocumentMeta* meta, size_t* count, size_t maxCount, void* user_data);
	char* (*get_document_text)(const DocumentMeta* meta, void* user_data);
	unsigned char* (*get_document_pdf_data)(const DocumentMeta* meta, void* user_data);
	int id;
	char* name;
	void* user_data;
};

static GSList *backends;

int sci_plugin_register(const char* name, DocumentMeta** (*fill_meta_in)(const DocumentMeta* meta, size_t* count, size_t maxCount, void* user_data),
						char* (*get_document_text_in)(const DocumentMeta* meta, void* user_data),
						unsigned char* (*get_document_pdf_data_in)(const DocumentMeta* meta, void* user_data), void* user_data)
{
	static int id_counter = 0;

	struct SciBackend* backend = g_malloc0(sizeof(*backend));

	backend->id = ++id_counter;
	backend->fill_meta = fill_meta_in;
	backend->get_document_text = get_document_text_in;
	backend->get_document_pdf_data = get_document_pdf_data_in;
	backend->name = g_strdup(name);
	backend->user_data = user_data;

	backends = g_slist_prepend(backends, backend);
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

	g_free(((struct SciBackend*)(element->data))->name);
	g_free(element->data);

	backends = g_slist_remove(backends, element->data);
}

DocumentMeta** sci_fill_meta(DocumentMeta* meta, size_t* count, size_t maxCount)
{
	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->fill_meta && (meta->backendId == backend->id || meta->backendId == 0))
		{
			DocumentMeta** newMeta = backend->fill_meta(meta, count, maxCount, backend->user_data);
			if(newMeta)
				return newMeta;
		}
	}
	sci_log(LL_WARN, "Unable to fill meta %s", __func__);
	count = 0;
	return NULL;
}

char* get_document_text(DocumentMeta* meta)
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

unsigned char* get_document_pdf_data(DocumentMeta* meta)
{
	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->get_document_pdf_data && (meta->backendId == backend->id || meta->backendId == 0))
		{
			unsigned char* data = backend->get_document_pdf_data(meta, backend->user_data);
			if(data)
				return data;
		}
	}
	sci_log(LL_WARN, "Unable to get pdf data %s", __func__);
	return NULL;
}
