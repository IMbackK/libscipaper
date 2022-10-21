#include "types.h"
#include <glib.h>
#include <stdio.h>
#include <assert.h>
#include "sci-log.h"

DocumentMeta* document_meta_new(void)
{
	return g_malloc0(sizeof(DocumentMeta));
}

DocumentMeta* document_meta_copy(const DocumentMeta* meta)
{
	DocumentMeta* copy = g_malloc0(sizeof(*copy));

	copy->doi = g_strdup(meta->doi);
	copy->url = g_strdup(meta->url);
	copy->year = meta->year;
	copy->publisher = g_strdup(meta->publisher);
	copy->volume = g_strdup(meta->volume);
	copy->pages = g_strdup(meta->pages);
	copy->author = g_strdup(meta->author);
	copy->title = g_strdup(meta->title);
	copy->journal = g_strdup(meta->journal);
	copy->issn = g_strdup(meta->issn);
	copy->keywords = g_strdup(meta->keywords);
	copy->downloadUrl = g_strdup(meta->downloadUrl);
	copy->abstract = g_strdup(meta->abstract);
	copy->searchText = g_strdup(meta->searchText);
	copy->backendId = meta->backendId;
	copy->hasFullText = meta->hasFullText;
	if(meta->backendData)
	{
		assert(meta->backend_data_copy_fn);
		copy->backendData = meta->backend_data_copy_fn(meta->backendData);
	}
	copy->backend_data_free_fn = meta->backend_data_free_fn;
	copy->backend_data_copy_fn = meta->backend_data_copy_fn;
	copy->compleatedLookup = meta->compleatedLookup;

	return copy;
}

void document_meta_free(DocumentMeta* meta)
{
	if(meta)
	{
		g_free(meta->doi);
		g_free(meta->url);
		g_free(meta->publisher);
		g_free(meta->volume);
		g_free(meta->pages);
		g_free(meta->author);
		g_free(meta->title);
		g_free(meta->journal);
		g_free(meta->issn);
		g_free(meta->keywords);
		g_free(meta->downloadUrl);
		g_free(meta->abstract);

		if(meta->backendData)
		{
			assert(meta->backend_data_free_fn);
			meta->backend_data_free_fn(meta->backendData);
		}

		g_free(meta->searchText);
		g_free(meta);
	}
}

void document_meta_print(const DocumentMeta* meta, bool info)
{
	sci_log(info ? LL_INFO : LL_DEBUG, "Document:\nDOI: %s\nTitle: %s\nAuthor: %s\nJournal: %s\nKeywords: %s\nAbstract: %s\n",
		meta->doi ? meta->doi : "", meta->title ? meta->title : "", meta->author ? meta->author : "",
		meta->journal ? meta->journal : "", meta->keywords ? meta->keywords : "",
		meta->abstract ? meta->abstract : "");
}

void document_meta_free_list(DocumentMeta** meta, size_t length)
{
	if(meta)
	{
		for(size_t i = 0; i < length; ++i)
			document_meta_free(meta[i]);
		g_free(meta);
	}
}

void pdf_data_free(PdfData* data)
{
	document_meta_free(data->meta);
	g_free(data->data);
	g_free(data);
}
