/*
 * scipaper.h
 * Copyright (C) Carl Philipp Klemm 2022 <carl@uvos.xyz>
 *
 * scipaper.h is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * scipaper.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <scipaper/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
Api for use by libscipaper users.
* @defgroup API User API
* This API allows you to lookup documents, find their full texts and grab PDF files.
* @{
*/

/**
 * @brief Sets the verbosity of sicpaper output
 *
 * @param verbosity The verbosity to set
 */
void sci_log_set_verbosity(loglevel_t verbosity);

/**
 * @brief Takes a DocumentMeta and tries to find maxCount documents that match the fields set in the
 * meta struct.
 *
 * @param meta A DocumentMeta struct with at least one value set.
 * If backendId == 0 all backends will be checked until one can identify the document otherwise the backend with the id backendId will be used
 * @param fill A pointer to a FillReqest struct that describes what fields are required by user, can be NULL for "don't care"
 * @param maxCount maximum number of documents to match
 * @param page if page is set > 0, the first page*maxCount entries are skipped and the subsequent results are returned instead
 * @return A RequestReturn, to be freed with request_return_free(), or NULL if none could be found
 */
RequestReturn* sci_fill_meta(const DocumentMeta* meta, const FillReqest* fill, size_t maxCount, size_t page);

/**
 * @brief Tries to find the metadata of the document with the given DOI
 *
 * @param doi DOI to search for
 * @param backendId The backend to use to find the DOI, or 0 for "any"
 * @return A filled DocumentMeta struct, to be freed with document_meta_free(), or NULL if none could be found
 */
DocumentMeta* sci_find_by_doi(const char* doi, int backendId);

/**
 * @brief Tries to find the metadata of the document with the given title
 *
 * @param title Title to search for
 * @return A filled DocumentMeta struct, to be freed with document_meta_free(), or NULL if none could be found
 */
DocumentMeta* sci_find_by_title(const char* title);

/**
 * @brief Tries to find the documents by a certain author
 *
 * @param author Author to search for
 * @param maxCount maximum number of documents to match
 * @return A RequestReturn, to be freed with request_return_free(), or NULL if none could be found
 */
RequestReturn* sci_find_by_author(const char* author, size_t maxCount);

/**
 * @brief Tries to find the documents by in a certain journal
 *
 * @param journal journal to search for
 * @param maxCount maximum number of documents to match
 * @return A list of filled RequestReturn, to be freed with request_return_free(), or NULL if none could be found
 */
RequestReturn* sci_find_by_journal(const char* journal, size_t maxCount);

/**
 * @brief Tries to get the full text of a certain document
 *
* @param meta A DocumentMeta struct with at least one value set if backendId == 0 all backends will be checked until one can identify the document otherwise the backend with the id backendId will be used
 * @return The full text of the document or NULL if the text is not available.
 */
char* sci_get_document_text(const DocumentMeta* meta);

/**
 * @brief Tries to get the PDF data of a certain document. Will give only the PDF of the first document that matches meta
 *
* @param meta A DocumentMeta struct with at least one value set if backendId == 0 all backends will be checked until one can identify the document otherwise the backend with the id backendId will be used
 * @return Raw data of the PDF document
 */
PdfData* sci_get_document_pdf_data(const DocumentMeta* meta);

/**
 * @brief Tries to get save the PDF of a certain document to disk. Will only save the first document that matches meta
 *
 * @param data A PdfData struct filled by sci_get_document_pdf_data
 * @return Raw data of the PDF document
 */
bool sci_save_pdf_to_file(const PdfData* data, const char* fileName);

/**
 * @brief Saves the PDF of a certain document to disk. Will only save the first document that matches meta
 *
* @param meta A DocumentMeta struct with at least one value set if backendId == 0 all backends will be checked until one can identify the document otherwise the backend with the id backendId will be used
 * @return true on success, false on failure
 */
bool sci_save_document_to_file(const DocumentMeta* meta, const char* fileName);

/**
 * @brief gives you an array describing each backend registered with libscipaper.
 *
 * @return a NULL terminated array of BackendInfo structs describing each backend, owned by libscipaper, do not free
 */
const BackendInfo** sci_get_all_backends(void);

/**
 * @brief gives you a BackendInfo struct describing the backend with the id
 *
 * @param id the id of the backend you want information on
 * @return a const BackendInfo struct containing information on the requested backend, owned by libscipaper, do not free
 */
const BackendInfo* sci_get_backend_info(int id);

/**
 * @brief gives you the id of the backend with a given name
 *
 * @param name the name of the backend
 * @return the id of the backend or 0 if it is not available
 */
int sci_backend_get_id_by_name(const char* name);

/**
 * @brief gives you the name of the backend with id
 * @param id the id of the backend you want the name for
 * @return a const string with the name of the backend, or "Invalid" if not valid, has static lifetime and shall not be freed.
 */
const char* sci_get_backend_name(int id);

/**
 * @brief gives you the number of backends currently registered
 * @return the number of backends currently registered
 */
size_t sci_get_backend_count(void);

/**
 * @brief Inits libscipaper, this function must be your first call to libscipaper, besides sci_get_version() and sci_log_set_verbosity()
 *
 * @param config_file An optional file name to a config ini file for libscipaper, or NULL
 * @param data An optional pointer to a keyfile in ram
 * @param length The length of the data at data
 * @return true on success false on failure
 */
bool sci_paper_init(const char* config_file, const char* data, size_t length);

/**
 * @brief Exits libscipaper, this function must be your final call to libscipaper,
 * it is an error to reinit libscipaper sci_paper_init() after executing sci_paper_exit()
 */
void sci_paper_exit(void);

/**
 * @brief get the version of scipaper in use
 * @return a const struct containing the version of scipaper in use. owned by libscipaper do not free
 */
const VersionFixed* sci_get_version(void);

/**
....
* @}
*/

#ifdef __cplusplus
}
#endif
