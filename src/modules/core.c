/*
 * core.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 * 
 * core.c is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * core.c is distributed in the hope that it will be useful, but
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
#define MODULE_NAME		"core"

/** Module information */
G_MODULE_EXPORT BackendInfo backend_info = {
	/** Name of the module */
	.name = MODULE_NAME,
};

DocumentMeta** core_fill_meta_in(const DocumentMeta* meta, size_t* count, size_t maxCount, void* userData)
{
	(void)meta;
	(void)maxCount;
	(void)userData;
	*count = 1;
	DocumentMeta** ret = g_malloc0(sizeof(*ret));
	ret[0] = document_meta_new();
	return ret;
}

char* core_get_document_text_in(const DocumentMeta* meta, void* userData)
{
	size_t count;
	(void)userData;
	return NULL;
}

unsigned char* core_get_document_pdf_data_in(const DocumentMeta* meta, void* userData)
{
	(void)meta;
	(void)userData;
	return NULL;
}


G_MODULE_EXPORT const gchar *sci_module_init(void** data);
const gchar *sci_module_init(void** data)
{
	data = GINT_TO_POINTER(sci_plugin_register(&backend_info, core_fill_meta_in, core_get_document_text_in, core_get_document_pdf_data_in, NULL));
	return NULL;
}

G_MODULE_EXPORT void sci_module_exit(void* data);
void sci_module_exit(void* data)
{
	sci_plugin_unregister(GPOINTER_TO_INT(data));
	(void)data;
}
