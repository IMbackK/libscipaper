#include <stdio.h>
#include <glib.h>
#include <stdbool.h>
#include "scipaper.h"
#include "sci-log.h"

void grab_wallauer()
{
	DocumentMeta* queryMeta = document_meta_new();
	queryMeta->author = g_strdup("Wallauer");

	size_t count;
	DocumentMeta** documents = sci_fill_meta(queryMeta, &count, 20);

	document_meta_free(queryMeta);

	if(documents)
	{
		sci_log(LL_INFO, "Found %zu documents", count);
		for(size_t i = 0; i < count; ++i)
		{
			if(!documents[i])
				continue;
			sci_log(LL_INFO, "Document found by %s:", sci_get_backend_info(documents[i]->backendId)->name);
			document_meta_print(documents[i], true);
		}
		document_meta_free_list(documents, count);
	}
	else
	{
		sci_log(LL_INFO, "Could not find any documents that matched query");
	}
	sci_log(LL_INFO, "");

}

void try_doi()
{
	DocumentMeta* meta = sci_find_by_doi("10.1002/ange.19410544309");

	if(meta)
	{
		sci_log(LL_INFO, "Found document for 10.1002/ange.19410544309");
		document_meta_print(meta, true);
	}
	else
	{
		sci_log(LL_INFO, "Could not find any documents that matched doi");
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
		sci_log(LL_ERR, "Coult not init libscipaper");
		return 1;
	}

	//grab_wallauer();
	try_doi();

	sci_paper_exit();
	return 0;
}
