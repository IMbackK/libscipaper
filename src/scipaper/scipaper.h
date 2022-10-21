#pragma once

#include <stdbool.h>
#include "types.h"

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
 * @param count A pointer where the nummber of found documents that match meta is stored
 * @param maxCount maximum number of documents to match
 * @return A list of filled DocumentMetas, to be freed with document_meta_free_list(), or NULL if none could be found
 */
DocumentMeta** sci_fill_meta(const DocumentMeta* meta, size_t* count, size_t maxCount);

/**
 * @brief Tries to find the metadata of the document with the given doi
 *
 * @param doi Doi to search for
 * @return A filled DocumentMeta struct, to be freed with document_meta_free(), or NULL if none could be found
 */
DocumentMeta* sci_find_by_doi(const char* doi);

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
 * @param count A pointer where the nummber of found documents that match meta is stored
 * @param maxCount maximum number of documents to match
 * @return A list of filled DocumentMetas, to be freed with document_meta_free_list(), or NULL if none could be found
 */
DocumentMeta** sci_find_by_author(const char* author, size_t* count, size_t maxCount);

/**
 * @brief Tries to find the documents by in a certain journal
 *
 * @param jornal journal to search for
 * @param count A pointer where the nummber of found documents that match meta is stored
 * @param maxCount maximum number of documents to match
 * @return A list of filled DocumentMetas, to be freed with document_meta_free_list(), or NULL if none could be found
 */
DocumentMeta** sci_find_by_journal(const char* jornal, size_t* count, size_t maxCount);

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
 * @brief gives you an array of BackendInfo structs describeing eatch backend registered with libscipaper
 *
 * @return an array of BackendInfo structs describeing eatch backend
 */
const BackendInfo** sci_get_all_backends(void);

/**
 * @brief gives you a BackendInfo struct describeing the backend with the id id
 *
 * @param id the id of the backend you want information on
 * @return a const BackendInfo struct containing information on the requested backend, owned by libscipaper, do not free
 */
const BackendInfo* sci_get_backend_info(int id);

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
 * @return true on sucess false on failure
 */
bool sci_paper_init(const char* config_file);

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
