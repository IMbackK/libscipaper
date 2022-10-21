#include <stdio.h>
#include <stdbool.h>
#include <glib.h>
#include "scipaper.h"

static void print_documents(DocumentMeta** documents, size_t count)
{
	printf("Found %zu documents:\n", count);
	for(size_t i = 0; i < count; ++i)
	{
		if(!documents[i])
			continue;
		char* documentString = document_meta_get_string(documents[i]);
		printf("Document found by %s:\n%s", sci_get_backend_info(documents[i]->backendId)->name, documentString);
		free(documentString);
	}
}

static void search_and_grab_wallauer_via_core()
{
	printf("Starting %s\n", __func__);
	int id = sci_backend_get_id_by_name("core");
	if(id == 0)
	{
		puts("core backend not available");
		return;
	}

	size_t count;
	DocumentMeta* queryMeta = document_meta_new();
	queryMeta->author = g_strdup("Wallauer");
	queryMeta->hasFullText = true;
	queryMeta->backendId = id;
	DocumentMeta** documents = sci_fill_meta(queryMeta, &count, 20);
	document_meta_free(queryMeta);

	if(documents)
	{
		print_documents(documents, count);

		puts("Getting text for first document:");
		char* text = sci_get_document_text(documents[0]);
		puts(text);
		free(text);
		document_meta_free_list(documents, count);
	}
	else
	{
		puts("Could not find any documents that matched query");
	}

}

static void search_wallauer(void)
{
	printf("Starting %s\n", __func__);
	DocumentMeta* queryMeta = document_meta_new();
	queryMeta->author = g_strdup("Wallauer");

	size_t count;
	DocumentMeta** documents = sci_fill_meta(queryMeta, &count, 20);

	document_meta_free(queryMeta);

	if(documents)
	{
		print_documents(documents, count);
		document_meta_free_list(documents, count);
	}
	else
	{
		puts("Could not find any documents that matched query");
	}
	puts("");
}

static void fill_meta_by_doi(void)
{
	printf("Starting %s\n", __func__);
	DocumentMeta* meta = sci_find_by_doi("10.1002/ange.19410544309");

	if(meta)
	{
		char* documentString = document_meta_get_string(meta);
		printf("Found document for 10.1002/ange.19410544309:\n%s", documentString);
		free(documentString);
	}
	else
	{
		puts("Could not find any documents that matched doi");
	}

	document_meta_free(meta);
}

int main(int argc, char** argv)
{
	const char* configFileName = NULL;
	if(argc > 1)
	{
		configFileName = argv[1];
		printf("%s using config file %s\n", __func__, configFileName);
	}

	sci_log_set_verbosity(LL_DEBUG);

	if(!sci_paper_init(configFileName))
	{
		printf("Coult not init libscipaper");
		return 1;
	}

	puts("Backends available:");
	const BackendInfo** backends = sci_get_all_backends();
	for(size_t i = 0; backends[i]; ++i)
	{
		char* cap = capability_flags_get_str(backends[i]->capabilities);
		printf("%zu:\t%s\tCapabilities: %s\n", i, backends[i]->name, cap);
		free(cap);
	}

	search_wallauer();
	fill_meta_by_doi();
	search_and_grab_wallauer_via_core();

	sci_paper_exit();
	return 0;
}
