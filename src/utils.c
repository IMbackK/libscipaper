#include "utils.h"

#include <curl/curl.h>
#include <sci-log.h>
#include <assert.h>

static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	GString* buffer = userp;
	g_string_append_len(buffer, (char*)contents, size * nmemb);
	return size * nmemb;
}

GString* wgetUrl(const char* url)
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
	ret = curl_easy_setopt(curlContext, CURLOPT_WRITEDATA, &buffer);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_SERVER_RESPONSE_TIMEOUT, 10);
	ret = curl_easy_perform(curlContext);
	curl_easy_cleanup(curlContext);
	if(ret != CURLE_OK)
	{
		sci_log(LL_ERR, "Could not load from %s curl retuned errno %i message: %s", url, ret, errorBuffer);
		return NULL;
	}

	return buffer;
}
