#pragma once
#include <glib.h>
#include "types.h"

/**
* @addtogroup MODAPI
* @{
*/

/**
 * @brief A key value Pair struct to be used wih G(S)Lists
 */
struct Pair
{
	char* key;
	char* value;
};

/**
 * @brief Frees a pair struct
 *
 * @param pair Pair to free
 */
void pair_free(struct Pair* pair);

/**
 * @brief Frees a pair struct
 * @param key the key of the key-value pair
 * @param value the value of the key-value pair
 * @return A newly allocated pair to be freed with pair_free
 */
struct Pair* pair_new(const char* key, const char* value);

/**
 * @brief Builds an url encoded query string that can be added to a url
 *
 * @param list const list of struct Pairs of key=value querys to add.
 * @return A newly allocated GString containing the query string
 */
GString* buildQuery(const GSList* list);

/**
 * @brief Get a pdf file va a http(s) GET request
 *
 * @param url The url to get
 * @return A newly PdfData struct or NULL if unsucessfull
 */
PdfData* wgetPdf(const char* url, int timeout);

/**
 * @brief Get the http data return as a string from a url via a http(s) GET request
 *
 * @param url The url to get
 * @return A newly allocated GString containing the data grabed from the url on sucess, or NULL on failure
 */
GString* wgetUrl(const char* url, int timeout);

/**
 * @brief Get the http data return as a string from a url via a http(s) POST request
 *
 * @param url The url to post
 * @param data null terminated string that contains the data to post to url
 * @return A newly allocated GString containing the data grabed from the url on sucess, or NULL on failure
 */
GString* wpostUrl(const char* url, const char* data, int timeout);

/**
 * @brief Create a json style entry string
 *
 * @param indent intent level to prepend
 * @param key json entry key
 * @param value json entry value
 * @param quote if true qoutes "" are placed around value
 * @param newline if true the returned string ends with a unix newline
 * @return A newly allocated GString containing the data grabed from the url on sucess, or NULL on failure
 */
GString* createJsonEntry(const int indent, const char* key, const char* value, bool quote, bool newline);

/**@}*/
