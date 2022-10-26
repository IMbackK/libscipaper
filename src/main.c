/*
 * main.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 *
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdbool.h>
#include <glib.h>
#include "scipaper.h"

static void print_documents(const RequestReturn* documents)
{
	printf("Found %zu documents:\n", documents->count);
	for(size_t i = 0; i < documents->count; ++i)
	{
		if(!documents->documents[i])
			continue;
		char* documentString = document_meta_get_string(documents->documents[i]);
		printf("Document found by %s:\n%s", sci_get_backend_info(documents->documents[i]->backendId)->name, documentString);
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

	DocumentMeta* queryMeta = document_meta_new();
	queryMeta->author = g_strdup("Wallauer");
	queryMeta->hasFullText = true;
	queryMeta->backendId = id;
	RequestReturn* documents = sci_fill_meta(queryMeta, NULL, 20, 0);
	document_meta_free(queryMeta);

	if(documents)
	{
		print_documents(documents);

		printf("Getting text for first document from: %s (%i)\n",
			   sci_get_backend_name(documents->documents[0]->backendId), documents->documents[0]->backendId);
		char* text = sci_get_document_text(documents->documents[0]);
		if(text)
			puts("got text!");
		free(text);

		for(size_t i = 0; i < documents->count; ++i)
		{
			PdfData* data = sci_get_document_pdf_data(documents->documents[i]);
			if(data)
			{
				puts("got got data! saveing..");
				char* fileName = g_strdup_printf("./%zu.pdf", i);
				bool ret = sci_save_pdf_to_file(data, fileName);
				g_free(fileName);
				if(ret)
					puts("saved");
				else
					puts("not saved");
			}
		}
		request_return_free(documents);
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

	RequestReturn* documents = sci_fill_meta(queryMeta, NULL, 20, 0);

	document_meta_free(queryMeta);

	if(documents)
	{
		print_documents(documents);
		request_return_free(documents);
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
	DocumentMeta* meta = sci_find_by_doi("10.1002/ange.19410544309", 0);

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

	if(!sci_paper_init(configFileName, NULL, 0))
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
