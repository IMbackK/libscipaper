#include <stdio.h>
#include <glib.h>
#include <stdbool.h>
#include "scipaper.h"
#include "sci-log.h"

int main(int argc, char** argv)
{
	if(!sci_paper_init(NULL))
	{
		sci_log(LL_ERR, "Coult not init libscipaper");
		return 1;
	}

	sci_log_set_verbosity(LL_DEBUG);

	DocumentMeta* queryMeta = document_meta_new();
	queryMeta->author = g_strdup("Wallauer");

	size_t count;
	DocumentMeta** documents = sci_fill_meta(queryMeta, &count, 20);

	if(documents)
	{
		sci_log(LL_INFO, "Found %zu documents", count);
		for(size_t i = 0; i < count; ++i)
		{
			sci_log(LL_INFO, "Document found by %s:", sci_get_backend_info(documents[i]->backendId)->name);
			document_meta_print(documents[i], true);
		}
		document_meta_free_list(documents, count);
	}
	else
	{
		sci_log(LL_INFO, "Could not find any documents that matched query");
	}

	sci_paper_exit();
	return 0;
}
