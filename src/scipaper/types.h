#pragma once
#include <time.h>
#include <stdbool.h>

/**
....
* @addtogroup API
*
* @{
*/

/**
 * @brief This struct contains the metadata of a paper
 */
typedef struct _DocumentMeta {
	char* doi; /**< The doi of the paper */
	char* url; /**< The url of the paper in the journal */
	struct timespec time; /**< Publication time of the paper */
	char* publisher; /**< Publisher time of the paper */
	char* volume; /**< Jornal volume where the paper apeard */
	char* pages; /**< Page(s) where the paper is to be found in the volume */
	char* author; /**< The author(s) of the paper */
	char* title; /**< The title of the paper */
	char* journal; /**< The journal in which the paper was published */
	char* keywords; /**< Keywords given by the author of the paper for the paper */
	char* pdfUrl; /**< Url where the pdf of the document can be found */
	char* abstract; /**< abstract of the document */

	int backendId; /**< The id of the backend that found the document, or the id that shal be tried to find the document */
	void* backendData; /**< Backend specific data */
	size_t backendDataLength; /**< Length of the backend specific data */
} DocumentMeta;

/**
 * @brief Mallocs a DocumentMeta struct and initalizes it
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
 * @param meta The DocumentMeta struct to free
 */
void document_meta_free(DocumentMeta* meta);

/**
 * @brief Prints a DocumentMeta to formated text output using the passed printf like printFn
 * @param info Print with INFO priority if true and DEBUG priority if false
 * @param meta The DocumentMeta struct to print
 */
void document_meta_print(const DocumentMeta* meta, bool info);

/**
 * @brief Frees a list/array of document metas
 * @param meta The DocumentMeta array to free
 */
void document_meta_free_list(DocumentMeta** meta, size_t length);

/**@}*/
