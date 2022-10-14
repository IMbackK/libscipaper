#include "types.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

DocumentMeta* document_meta_new(void)
{
	return g_malloc0(sizeof(DocumentMeta));
}

DocumentMeta* document_meta_copy(const DocumentMeta* meta)
{
	DocumentMeta* copy = g_malloc0(sizeof(*copy));

	copy->doi = g_strdup(meta->doi);
	copy->url = g_strdup(meta->url);
	copy->time = meta->time;
	copy->publisher = g_strdup(meta->publisher);
	copy->volume = g_strdup(meta->volume);
	copy->pages = g_strdup(meta->pages);
	copy->author = g_strdup(meta->author);
	copy->title = g_strdup(meta->title);
	copy->journal = g_strdup(meta->journal);
	copy->keywords = g_strdup(meta->keywords);
	copy->pdfUrl = g_strdup(meta->pdfUrl);
	copy->backendId = meta->backendId;
	if(meta->backendData)
	{
		copy->backendData = g_malloc0(meta->backendDataLength);
		memcpy(copy->backendData, meta->backendData, meta->backendDataLength);
	}
	copy->backendDataLength = meta->backendDataLength;

	return copy;
}

void document_meta_free(DocumentMeta* meta)
{
	g_free(meta->doi);
	g_free(meta->url);
	g_free(meta->publisher);
	g_free(meta->volume);
	g_free(meta->pages);
	g_free(meta->author);
	g_free(meta->title);
	g_free(meta->journal);
	g_free(meta->keywords);
	g_free(meta->pdfUrl);
	g_free(meta->bibtex);
	g_free(meta->backendData);
	g_free(meta);
}

void document_meta_free_list(DocumentMeta** meta, size_t length)
{
	for(size_t i = 0; i < length; ++i)
		document_meta_free(meta[i]);
	g_free(meta);
}
