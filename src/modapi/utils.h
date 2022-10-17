#pragma once
#include <glib.h>

/**
 * @brief Get the http data return as a string from a url
 *
 * @param url The url to get
 * @return A GString containing the data grabed from the url on sucess, or NULL on failure
 */
GString* wgetUrl(const char* url);
