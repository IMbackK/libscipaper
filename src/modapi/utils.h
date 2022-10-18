#pragma once
#include <glib.h>

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
 * @brief Builds an escaped query string for a url
 *
 * @param list list of struct Pairs of key=value querys to add, ownership of this list is taken over by buildQuery and will be freed by it.
 * @return A newly allocated GString containing the query string
 */
GString* buildQuery(const GSList* list);

/**
 * @brief Get the http data return as a string from a url
 *
 * @param url The url to get
 * @return A newly allocated GString containing the data grabed from the url on sucess, or NULL on failure
 */
GString* wgetUrl(const char* url, int timeout);
