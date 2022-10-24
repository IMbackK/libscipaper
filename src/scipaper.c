#include "scipaper.h"

#include <stdbool.h>
#include <assert.h>
#include <glib.h>

#include "sci-log.h"
#include "sci-conf.h"
#include "sci-modules.h"
#include "scipaper.h"

bool sci_paper_init(const char* config_file)
{
	sci_log_open("libscipaper", LOG_USER, SCI_LOG_STDERR);

	if(!sci_conf_init(config_file))
		return false;

	if(!sci_modules_init())
		return false;

	return true;
}

void sci_paper_exit(void)
{
	sci_modules_exit();
	sci_conf_exit();
	size_t backendCount = sci_get_backend_count();
	if(backendCount != 0)
		sci_log(LL_WARN, "%zu backend(s) have failed to unregister!!!", backendCount);
}

DocumentMeta* sci_find_by_doi(const char* doi, int backendId)
{
	DocumentMeta meta = {0};
	meta.doi = (char*)doi;
	meta.backendId = backendId;
	size_t count;
	DocumentMeta** documentMetas = sci_fill_meta(&meta, NULL, &count, 1, 0);
	if(documentMetas)
	{
		DocumentMeta* meta = documentMetas[0];
		g_free(documentMetas);
		return meta;
	}
	else
	{
		return NULL;
	}
}

DocumentMeta** sci_find_by_author(const char* author, size_t* count, size_t maxCount)
{
	DocumentMeta meta = {0};
	meta.author = (char*)author;
	return sci_fill_meta(&meta, NULL, count, maxCount, 0);
}

DocumentMeta* sci_find_by_title(const char* title)
{
	DocumentMeta meta = {0};
	meta.title = (char*)title;
	size_t count;
	DocumentMeta** documentMetas = sci_fill_meta(&meta, NULL, &count, 1, 0);
	if(documentMetas)
	{
		DocumentMeta* meta = documentMetas[0];
		g_free(documentMetas);
		return meta;
	}
	else
	{
		return NULL;
	}
}

DocumentMeta** sci_find_by_journal(const char* jornal, size_t* count, size_t maxCount)
{
	DocumentMeta meta = {0};
	meta.journal = (char*)jornal;
	return sci_fill_meta(&meta, NULL, count, maxCount, 0);
}

bool sci_save_pdf_to_file(const PdfData* data, const char* fileName)
{
	if(!data || !data->data)
		return false;

	GError *error = NULL;
	if(!g_file_set_contents_full(fileName, (gchar*)data->data, data->length, G_FILE_SET_CONTENTS_NONE, 0666, &error))
	{
		sci_log(LL_ERR, "%s: %s", __func__, error->message);
		g_error_free(error);
		return false;
	}

	return true;
}

bool sci_save_document_to_file(const DocumentMeta* meta, const char* fileName)
{
	PdfData* data = sci_get_document_pdf_data(meta);
	if(!data)
		return false;
	return sci_save_pdf_to_file(data, fileName);
}
