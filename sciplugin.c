#include "sciplugin.h"

#include <glib.h>

#include "sci-log.h"

typedef struct SciBackend
{
	DocumentMeta* (*fill_meta)(const DocumentMeta* meta);
	char* (*get_document_text)(DocumentMeta* meta);
	unsigned char* (*get_document_pdf_data)(DocumentMeta* meta);
	int id;
};

static GSList *uis;

int sci_plugin_register(DocumentMeta* (*fill_meta_in)(const DocumentMeta* meta),
						char* (*get_document_text_in)(DocumentMeta* meta),
						unsigned char* (*get_document_pdf_data_in)(DocumentMeta* meta))
{
	static int id_counter = 0;

	struct SciBackend* backend = g_malloc0(sizeof(*backend));

	backend->id = ++id_counter;
	backend->fill_meta = fill_meta_in;
	backend->get_document_text = get_document_text_in;
	backend->get_document_pdf_data = get_document_pdf_data_in;
}

void sci_plugin_unregister(int id)
{

}
