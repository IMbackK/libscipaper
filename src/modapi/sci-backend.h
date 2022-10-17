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
int sci_plugin_register(const BackendInfo* backend_info, DocumentMeta** (*fill_meta_in)(const DocumentMeta*, size_t*, size_t, void*),
						char* (*get_document_text_in)(const DocumentMeta*, void*),
						unsigned char* (*get_document_pdf_data_in)(const DocumentMeta*, void*), void* user_data);

/**
 * @brief Unregisters a backend, must be called before the backend exits
 * @param id the backend id to unreigster.
 */
void sci_plugin_unregister(int id);

/**@}*/
