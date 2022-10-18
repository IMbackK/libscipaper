#include "utils.h"

#include <curl/curl.h>
#include <sci-log.h>
#include <assert.h>
#include <stdbool.h>

void pair_free(struct Pair* pair)
{
	g_free(pair->key);
	g_free(pair->value);
	g_free(pair);
}

struct Pair* pair_new(const char* key, const char* value)
{
	struct Pair* ret = g_malloc0(sizeof(*ret));
	ret->key = g_strdup(key);
	ret->value = g_strdup(value);
	return ret;
}

GString* buildQuery(const GSList* list)
{
	GString* string = g_string_new("?");
	for(const GSList* element = list; element; element = element->next)
	{
		const struct Pair *pair = element->data;
		g_string_append(string, pair->key);
		g_string_append_c(string, '=');
		char* escapedValue = g_uri_escape_string(pair->value, ",", false);
		g_string_append(string, escapedValue);
		if(element->next)
			g_string_append_c(string, '&');
	}
	return string;
}

static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	GString* buffer = userp;
	g_string_append_len(buffer, (char*)contents, size * nmemb);
	return size * nmemb;
}

GString* wgetUrl(const char* url, int timeout)
{
	CURL* curlContext = curl_easy_init();
	if(!curlContext)
	{
		sci_log(LL_ERR, "Utils: Could not init curl");
		return NULL;
	}

	char errorBuffer[CURL_ERROR_SIZE] = "";
	GString* buffer = g_string_new(NULL);
	CURLcode ret;

	ret = curl_easy_setopt(curlContext, CURLOPT_ERRORBUFFER, errorBuffer);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_URL, url);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_WRITEFUNCTION, writeCallback);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_WRITEDATA, buffer);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_TIMEOUT, timeout);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_SERVER_RESPONSE_TIMEOUT, timeout/3);
	assert(ret == CURLE_OK);
	ret = curl_easy_perform(curlContext);
	curl_easy_cleanup(curlContext);
	if(ret != CURLE_OK)
	{
		sci_log(LL_ERR, "Could not load from %s curl retuned errno %i\n%s", url, ret, errorBuffer);
		g_string_free(buffer, true);
		return NULL;
	}

	return buffer;
}
