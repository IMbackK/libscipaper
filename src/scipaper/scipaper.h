/*
 * scipaper.h
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
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
* This api allows you to lookup documents, find thair full texts and grab pdf files.
* @{
*/

/**
 * @brief Sets the verbosity of sicpaper output
 *
 * @param verbosity The versboity to set
 */
void sci_log_set_verbosity(loglevel_t verbosity);

/**
 * @brief Takes a DocumentMeta and trys to find maxCount documents that match the fileds set in the
 * meta struct.
 *
 * @param meta A DocumentMeta struct with at least one value set.
 * If backendId == 0 all backends will be checked until one can indentify the document otherwise the backend with the id backendId will be used
 * @param fill A pointer to a FillReqest struct that discribes what fields are required by user, can be NULL for "dont care"
 * @param maxCount maximum number of documents to match
 * @param page if page is set > 0, the first page*maxCount entries are skipped and the subsiquent results are returned instead
 * @return A RequestReturn, to be freed with request_return_free(), or NULL if none could be found
 */
RequestReturn* sci_fill_meta(const DocumentMeta* meta, const FillReqest* fill, size_t maxCount, size_t page);

/**
 * @brief Tries to find the metadata of the document with the given doi
 *
 * @param doi Doi to search for
 * @param backendId The backend to use to find the doi, or 0 for "any"
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
 * @param jornal journal to search for
 * @param maxCount maximum number of documents to match
 * @return A list of filled RequestReturn, to be freed with request_return_free(), or NULL if none could be found
 */
RequestReturn* sci_find_by_journal(const char* jornal, size_t maxCount);

/**
 * @brief Tries to get the full text of a certain document
 *
* @param meta A DocumentMeta struct with at least one value set if backendId == 0 all backends will be checked until one can indentify the document otherwise the backend with the id backendId will be used
 * @return The full text of the document or NULL if the text is not available.
 */
char* sci_get_document_text(const DocumentMeta* meta);

/**
 * @brief Tries to get the pdf data of a certain document. Will give only the pdf of the first document that matches meta
 *
* @param meta A DocumentMeta struct with at least one value set if backendId == 0 all backends will be checked until one can indentify the document otherwise the backend with the id backendId will be used
 * @return Raw data of the pdf document
 */
PdfData* sci_get_document_pdf_data(const DocumentMeta* meta);

/**
 * @brief Tries to get save the pdf of a certain document to disk. Will only save the first document that matches meta
 *
 * @param data A PdfData struct filled by sci_get_document_pdf_data
 * @return Raw data of the pdf document
 */
bool sci_save_pdf_to_file(const PdfData* data, const char* fileName);

/**
 * @brief Saves the pdf of a certain document to disk. Will only save the first document that matches meta
 *
* @param meta A DocumentMeta struct with at least one value set if backendId == 0 all backends will be checked until one can indentify the document otherwise the backend with the id backendId will be used
 * @return true on sucess, false on failure
 */
bool sci_save_document_to_file(const DocumentMeta* meta, const char* fileName);

/**
 * @brief gives you an array describeing eatch backend registered with libscipaper.
 *
 * @return a NULL terminated array of BackendInfo structs describeing eatch backend, owend by libscipaper, do not free
 */
const BackendInfo** sci_get_all_backends(void);

/**
 * @brief gives you a BackendInfo struct describeing the backend with the id
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
 * @return a const string with the name of the backend, has static lifetime and shal not be freed
 */
const char* sci_get_backend_name(int id);

/**
 * @brief gives you the nummber of backends currently registered
 * @return the number of backends currently registered
 */
size_t sci_get_backend_count(void);

/**
 * @brief Inits libscipaper, this function must be your first call to libscipaper
 *
 * @param config_file An optional file name to a config ini file for libscipaper, or NULL
 * @param data An optional pointer to a keyfile in ram
 * @param length The length of the data at data
 * @return true on sucess false on failure
 */
bool sci_paper_init(const char* config_file, const char* data, size_t length);

/**
 * @brief Exits libscipaper, this function must be your final call to libscipaper,
 * it is an error to reinit libscipaper sci_paper_init() after executig sci_paper_exit()
 */
void sci_paper_exit(void);

/**
....
* @}
*/

#ifdef __cplusplus
}
#endif
