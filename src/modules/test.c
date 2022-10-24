/*
 * test.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 * 
 * test.c is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * test.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include "sci-modules.h"
#include "sci-log.h"
#include "sci-backend.h"
#include "types.h"
#include "scipaper.h"

/** Module name every module is required to have this*/
#define MODULE_NAME		"test"

/** Module information */
G_MODULE_EXPORT BackendInfo backend_info = {
	/** Name of the module */
	.name = MODULE_NAME,
};

DocumentMeta** test_fill_meta_in(const DocumentMeta* meta, size_t* count, size_t maxCount, size_t page, void* userData)
{
	(void)meta;
	(void)maxCount;
	(void)userData;
	(void)page;

	if(maxCount == 0)
	{
		sci_module_log(LL_WARN, "A request for 0 results was given");
		return NULL;
	}

	*count = 1;
	DocumentMeta** ret = g_malloc0(sizeof(*ret));
	ret[0] = document_meta_new();
	return ret;
}

char* test_get_document_text_in(const DocumentMeta* meta, void* userData)
{
	size_t count;
	(void)userData;
	DocumentMeta** results = sci_fill_meta(meta, NULL, &count, 1, 0);
	if(results)
	{
		document_meta_free_list(results, count);
		return g_strdup("Quia blanditiis omnis aliquam pariatur. Aut est reiciendis omnis et. \
		Placeat ea officia laborum eum vel adipisci deleniti. Earum aut eveniet minima libero itaque nisi quia.");
	}
	else
	{
		return NULL;
	}
}

static PdfData* test_get_document_pdf_data_in(const DocumentMeta* meta, void* userData)
{
	(void)meta;
	(void)userData;
	return NULL;
}


G_MODULE_EXPORT const gchar *sci_module_init(void** data);
const gchar *sci_module_init(void** data)
{
	data = GINT_TO_POINTER(sci_plugin_register(&backend_info, test_fill_meta_in, test_get_document_text_in, test_get_document_pdf_data_in, NULL));
	sci_module_log(LL_DEBUG, "works");
	return NULL;
}

G_MODULE_EXPORT void sci_module_exit(void* data);
void sci_module_exit(void* data)
{
	sci_plugin_unregister(GPOINTER_TO_INT(data));
	(void)data;
}
