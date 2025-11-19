/*
 * types.h
 * Copyright (C) Carl Philipp Klemm 2022 <carl@uvos.xyz>
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
#include <time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
....
* @addtogroup API
*
* @{
*/

/**
 * @brief Severity of loglevels
 */
typedef enum {
	LL_NONE = 0,			/**< No logging at all */
	LL_CRIT = 1,			/**< Critical error */
	LL_ERR = 2,			 	/**< Error */
	LL_WARN = 3,			/**< Warning */
	LL_DEFAULT = LL_WARN,	/**< Default log level */
	LL_INFO = 4,			/**< Informational message */
	LL_DEBUG = 5			/**< Useful when debugging */
} loglevel_t;

/**
 * @brief Flags that describe what a backend can do
 */
typedef enum {
	SCI_CAP_FILL = 1,			/**< Backend can fill DocumentMeta structs*/
	SCI_CAP_GET_TEXT = (1<<1),	/**< Backend can get full text of documents*/
	SCI_CAP_GET_PDF = (1<<2),	/**< Backend can get pdfs of documents*/
} capability_flags_t;

/**
 * @brief Sorting direction of output
 */
typedef enum {
	SCI_SORT_INVALID = -1,
	SCI_SORT_RELEVANCE = 0,		/**< This effectively lets the backend choose a sorting direction*/
	SCI_SORT_REFERANCES,	/**< Sort by most referanced work to least referanced work*/
	SCI_SORT_OLDETST,	/**< Sort by publication date oldest to newest*/
	SCI_SORT_NEWETST,	/**< Sort by publication date newest to oldest*/
} sorting_mode_t;

const char* sorting_mode_name(sorting_mode_t mode);

/**
 * @brief returns the capabilities flags as a human readable string.
 * @param capabilities Print with INFO priority if true and DEBUG priority if false
 * @return A newly allocated string stating the flags
 */
char* capability_flags_get_str(capability_flags_t capabilities);

/**
 * @brief This struct contains the version of libscipaper in use
 */
typedef struct _VersionFixed {
	unsigned int major;
	unsigned int minor;
	unsigned int patch;
} VersionFixed;

/**
 * @brief Backend information struct
 */
typedef struct _BackendInfo {
	const char *const name; /**< Name of the plugin */
	capability_flags_t capabilities; /**< Flags that describe what a backend can do */
} BackendInfo;

/**
 * @brief This bitfield tells libscipaper what fields you require to have filled.
 * libscipaper will try each of its backends in sequence until
 */
typedef struct _FillReqest {
	bool doi:1; /**< The DOI of the paper */
	bool url:1; /**< The URL of the paper in the journal */
	bool year:1; /**< Publication year of the paper */
	bool publisher:1; /**< Publisher of the paper */
	bool volume:1; /**< Journal volume where the paper appeared */
	bool pages:1; /**< Page(s) where the paper is to be found in the volume */
	bool author:1; /**< The author(s) of the paper */
	bool title:1; /**< The title of the paper */
	bool journal:1; /**< The journal in which the paper was published */
	bool issn:1; /**< The journal issn in which the paper was published */
	bool keywords:1; /**< Keywords given by the author of the paper for the paper */
	bool downloadUrl:1; /**< URL where the full text of the document can be found */
	bool abstract:1; /**< abstract of the document */
	bool references:1; /**< how often the article has been cited */
} FillReqest;

/**
 * @brief This struct contains the metadata of a paper, must be created via document_meta_new() and freed via document_meta_free()
 */
typedef struct _DocumentMeta {
	//To be filled by user for query or by backend as a result
	char* doi; /**< The DOI of the paper */
	char* url; /**< The URL of the paper in the journal */
	unsigned long year; /**< Publication year of the paper */
	char* publisher; /**< Publisher of the paper */
	char* volume; /**< Journal volume where the paper appeared */
	char* pages; /**< Page(s) where the paper is to be found in the volume */
	char* author; /**< The author(s) of the paper */
	char* title; /**< The title of the paper */
	char* journal; /**< The journal in which the paper was published */
	char* issn; /**< The journal issn in which the paper was published */
	char* keywords; /**< Keywords given by the author of the paper for the paper */
	char* downloadUrl; /**< URL where the full text of the document can be found */
	char* abstract; /**< abstract of the document */
	int references; /**< how often the article has been cited */

	char* searchText; /**< freeform text to search for in backends */
	bool hasFullText; /**< a hint that document has full text available */

	int backendId; /**< The id of the backend that found the document, or the id that shall be tried to find the document */

	//To be filled by backend
	void* backendData; /**< Backend specific data, not to be used by clients*/
	void (*backend_data_free_fn)(void*); /**< Function to free backend specific data, not to be used by clients*/
	void* (*backend_data_copy_fn)(void*); /**< Function to deep copy backend specific data, not to be used by clients*/

	//Filled by libscipaper core
	bool compleatedLookup; /**< Entry lookup completed */
} DocumentMeta;

/**
 * @brief Mallocs a DocumentMeta struct and initializes it
 * @return A newly allocated DocumentMeta struct
 */
DocumentMeta* document_meta_new(void);

/**
 * @brief Dose a deep copy of a DocumentMeta struct
 * @param meta The DocumentMeta struct to copy
 * @return A newly allocated copy of the meta struct
 */
DocumentMeta* document_meta_copy(const DocumentMeta* meta);

/**
 * @brief Frees a document meta struct
 * @param meta The DocumentMeta struct to free, it is safe to pass NULL here
 */
void document_meta_free(DocumentMeta* meta);

/**
 * @brief Adds the fields set in source but not in target to target
 * @param target The DocumentMeta struct where the fields of source are to be added to
 * @param source The DocumentMeta struct where to get the fields from
 */
void document_meta_combine(DocumentMeta* target, const DocumentMeta* source);

/**
 * @brief Creates a human readable string describing a DocumentMeta
 * @param meta The DocumentMeta struct to print
 * @return a newly allocated string containing the description
 */
char* document_meta_get_string(const DocumentMeta* meta);

/**
 * @brief Get string containing json data of the supplied DoucmentMeta
 * @param meta The DocumentMeta struct to save into the string
 * @param fullText Optionally the full text associated with the DocumentMeta
 * @param length length of the returned string containing the json data
 * @return A newly allocated string containing the json data
 */
char* document_meta_get_json(const DocumentMeta* meta, const char* fullText, size_t* length);

char* document_meta_get_json_only_fillrq(const DocumentMeta* meta, const FillReqest rq, const char* fullText, size_t* length);

/**
 * @brief create a DocumentMeta from json data saved by document_meta_get_json()
 * @param jsonFile a c string containing the json data
 * @return A newly allocated DocumentMeta containing metadata contained in the file, or NULL if invalid
 */
DocumentMeta* document_meta_load_from_json(char* jsonFile);

/**
 * @brief create a DocumentMeta from json file saved by document_meta_save()
 * @param jsonFileName a filename of a json file to load
 * @return A newly allocated DocumentMeta containing metadata contained in the file, or NULL if invalid
 */
DocumentMeta* document_meta_load_from_json_file(const char* jsonFileName);

/**
 * @brief create a DocumentMeta from json file saved by document_meta_save()
 * @param jsonFileName a filename of a json file to load
 * @return A newly allocated string with the text in the given file, or NULL if invalid
 */
char* document_meta_load_full_text_from_json_file(const char* jsonFileName);

/**
 * @brief Get string containing biblatex entry of the supplied DoucmentMeta
 * @param meta The DocumentMeta struct to save into the string
 * @param length length of the returned string containing the biblatex data
 * @param type string with type of biblatex entry or NULL for default/dont care
 * @return A newly allocated string containing the biblatex entry
 */
char* document_meta_get_biblatex(const DocumentMeta* meta, size_t* length, const char* type);

/**
 * @brief Saves a DocumentMeta to disk
 * @param fileName The file name under which to save the DocumentMeta
 * @param meta The DocumentMeta struct to save to disk
 * @param fullText Optionally the full text associated with the DocumentMeta
 * @return true on success false on failure
 */
bool document_meta_save(const char* fileName, const DocumentMeta* meta, const char* fullText);

bool document_meta_save_only_fillrq(const char* fileName, const DocumentMeta* meta, const FillReqest fq, const char* fullText);

/**
 * @brief Frees a list/array of document metas
 * @param meta The DocumentMeta array to free, it is safe to pass NULL here
 */
void document_meta_free_list(DocumentMeta** meta, size_t length);

/**
 * @brief Compares to document metas and returns if they are equal.
 * This function doesn't tell you if a and b refer to the same document, it only tells you if the DocumentMetas are bitwise the same
 * @param a A DocumentMeta struct
 * @param b Another DocumentMeta struct to compare to a
 * @return true if a and b are equal, false otherwise
 */
bool document_meta_is_equal(const DocumentMeta* a, const DocumentMeta* b);

/**
 * @brief This struct is details the result of a metadata search.
 * it contains a series of DocumentMeta structs as well as information about the query.
 * This struct must be freed with request_return_free()
 */
typedef struct _RequestReturn {
	DocumentMeta** documents; /**< A array of document meta structs detailing the search results */
	size_t count; /**< The length of the DocumentMeta array */
	size_t maxCount; /**< The maximum number of search results to be presented, as requested by the interface user */
	size_t page; /**< The page that was requested */
	size_t totalCount; /**< The total number of search results found by the backend, 0 if this information is not supported by the backend*/
} RequestReturn;

/**
 * @brief Allocates a empty RequestReturn struct
 * @param count number of DocumentMeta structs this struct contains
 * @param maxCount maximum number of DocumentMeta structs requested by interface user
 * @return a newly allocated RequestReturn struct, to be freed with request_return_free()
 */
RequestReturn* request_return_new(size_t count, size_t maxCount);

/**
 * @brief Frees a RequestReturn struct
 * @param reqRet The RequestReturn to free, it is safe to pass NULL here
 */
void request_return_free(RequestReturn* reqRet);

/**
 * @brief This struct contains the raw data of a PDF document
 */
typedef struct _PdfData
{
	unsigned char* data; /**< Raw data */
	size_t length; /**< Length of data */
	DocumentMeta* meta; /**< Meta data of the document that the PDF belongs to */
} PdfData;

/**
 * @brief Frees a PdfData struct
 * @param data The PdfData struct to free, it is safe to pass NULL here
 */
void pdf_data_free(PdfData* data);

/**@}*/

#ifdef __cplusplus
}
#endif
