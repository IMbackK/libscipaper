#pragma once
#include <time.h>

typedef struct _DocumentMeta {
	char* doi;
	char* url;
	struct timespec time;
	char* publisher;
	char* volume;
	char* pages;
	char* author;
	char* title;
	char* journal;
	char* keywords;
	char* pdfUrl;
	char* bibtex;

	int backendId;
	void* backendData;
	size_t backendDataLength;
} DocumentMeta;

DocumentMeta* document_meta_new(void);
DocumentMeta* document_meta_copy(const DocumentMeta* meta);
void document_meta_free(DocumentMeta* meta);
void document_meta_free_list(DocumentMeta** meta, size_t length);
