#include "types.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

DocumentMata* document_meta_copy(const DocumentMata* meta)
{
	DocumentMata* copy = g_malloc0(sizeof(*copy));

	copy->doi = g_strdup(meta->doi);
	copy->url = g_strdup(meta->url);
	copy->time = meta->time;
	copy->publisher = g_strdup(meta->publisher);
	copy->volume = g_strdup(meta->volume);
	copy->pages = g_strdup(meta->pages);
	copy->author = g_strdup(meta->author);
	copy->title = g_strdup(meta->title);
	copy->jornal = g_strdup(meta->jornal);
	copy->keywords = g_strdup(meta->keywords);
	copy->pdfUrl = g_strdup(meta->pdfUrl);
	copy->filledByBackend = meta->filledByBackend;
	if(meta->backendData)
	{
		copy->backendData = g_malloc0(meta->backendDataLength);
		memcpy(copy->backendData, meta->backendData, meta->backendDataLength);
	}
	copy->backendDataLength = meta->backendDataLength;

	return copy;
}

DocumentMata* document_meta_free(const DocumentMata* meta)
{
	g_free(meta->doi);
	g_free(meta->url);
	g_free(meta->publisher);
	g_free(meta->volume);
	g_free(meta->pages);
	g_free(meta->author);
	g_free(meta->title);
	g_free(meta->jornal);
	g_free(meta->keywords);
	g_free(meta->pdfUrl);
	g_free(meta->bibtex);
	g_gree(meta->backendData);
	g_free(meta);
}
