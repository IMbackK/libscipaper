/*
 * types.h
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 *
 * types.h is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * types.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "types.h"
/**
Api for use by libscipaper modules.
* @defgroup MODAPI Module API
* This api is to be used by implemeters of libscipaper modules to expose the various features of webapis used as sources of metadata as well as full text information.
* @{
*/

/**
 * @brief Registers a backend, not all functions have to be regisered for eatch backend, pass NULL for unwanted functions.
 * @param backend_info a module_info_struct descibeing the backend in question. it is expected that this struct have static lifetime.
 * @param fill_meta_in a function pointer to a function that takes a DocumentMeta struct and fills it. See sci_fill_meta() for details on parameters
 * @param get_document_text_in a function pointer to a function that gets the full text for a document. See sci_get_document_text() for details on parameters
 * @param get_document_pdf_data_in a function pointer to a function that gets the pdf data for a document. See sci_get_document_pdf_data() for details on parameters
 * @param user_data a point for context that will be passed to fill_meta_in, get_document_text_in, and get_document_pdf_data_in when called
 * @return backend id that is to be given in DocumentMeta backendId as well as input for sci_plugin_unregister()
 */
int sci_plugin_register(const BackendInfo* backend_info, RequestReturn* (*fill_meta_in)(const DocumentMeta*, size_t, sorting_mode_t, size_t, void*),
						char* (*get_document_text_in)(const DocumentMeta*, void*),
						PdfData* (*get_document_pdf_data_in)(const DocumentMeta*, void*), void* user_data);

/**
 * @brief Unregisters a backend, must be called before the backend exits
 * @param id the backend id to unreigster.
 */
void sci_plugin_unregister(int id);

/**@}*/
