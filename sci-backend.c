#include "sciplugin.h"

#include <glib.h>

#include "sci-log.h"

typedef struct SciBackend
{
	DocumentMeta** (*fill_meta)(const DocumentMeta* meta, size_t* count, size_t maxCount);
	char* (*get_document_text)(const DocumentMeta* meta);
	unsigned char* (*get_document_pdf_data)(const DocumentMeta* meta);
	int id;
	char* name;
};

static GSList *backends;

int sci_plugin_register(const char* name, DocumentMeta** (*fill_meta_in)(const DocumentMeta* meta, size_t* count, size_t maxCount),
						char* (*get_document_text_in)(const DocumentMeta* meta),
						unsigned char* (*get_document_pdf_data_in)(const DocumentMeta* meta))
{
	static int id_counter = 0;

	struct SciBackend* backend = g_malloc0(sizeof(*backend));

	backend->id = ++id_counter;
	backend->fill_meta = fill_meta_in;
	backend->get_document_text = get_document_text_in;
	backend->get_document_pdf_data = get_document_pdf_data_in;
	backend->name = g_strdup(name);

	backends = g_slist_prepend(backends, backend);
	return id_counter;
}

void sci_plugin_unregister(int id)
{
	GSList *element;
	for(element = backends; element; element = element->next)
	{
		SciBackend *last_backend = (SciBackend*)element->data;
		if(last_backend->id == id)
			break;
	}
	if (!element)
		sci_log(LL_WARN, "Trying to remove non-existing comm backend with id %d", id);

	g_free(((SciBackend*)(element->data))->name);
	g_free(element->data);

	backends = g_slist_remove(backends, element->data);
}

DocumentMeta** sci_fill_meta(DocumentMeta* meta, size_t* count, size_t maxCount)
{
	bool backend_avail = false;
	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->fill_meta)
		{
			DocumentMeta* newMeta = backend->fill_meta(meta, count, maxCount);
			if(newMeta)
				return newMeta;
		}
	}
	if(!backend_avail)
		sci_log(LL_WARN, "No backend for %s", __func__);
	else
		sci_log(LL_WARN, "Unable to fill meta %s", __func__);
	count = 0;
	return NULL;
}

char* get_document_text(DocumentMeta* meta)
{
	bool backend_avail = false;
	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->get_document_text)
		{
			char* text = backend->get_document_text(meta);
			if(text)
				return text;
		}
	}
	if(!backend_avail)
		sci_log(LL_WARN, "No backend for %s", __func__);
	else
		sci_log(LL_WARN, "Unable to get text %s", __func__);
	return NULL;
}

unsigned char* get_document_pdf_data(DocumentMeta* meta)
{
	bool backend_avail = false;
	for(GSList *element = backends; element; element = element->next)
	{
		struct SciBackend* backend = element->data;
		if(backend->get_document_pdf_data)
		{
			char* data = backend->get_document_pdf_data(meta);
			if(data)
				return data;
		}
	}
	if(!backend_avail)
		sci_log(LL_WARN, "No backend for %s", __func__);
	else
		sci_log(LL_WARN, "Unable to get pdf data %s", __func__);
	return NULL;
}
