/*
 * utils.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 *
 * utils.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * utils.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
		g_free(escapedValue);
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

static GString* wgetUrlUa(const char* url, int timeout)
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
	ret = curl_easy_setopt(curlContext, CURLOPT_TIMEOUT, (long)timeout);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:106.0) Gecko/20100101 Firefox/106.0");
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_SERVER_RESPONSE_TIMEOUT, (long)timeout/3);
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

PdfData* wgetPdf(const char* url, int timeout)
{
	GString *response = wgetUrlUa(url, timeout);

	PdfData* pdfData = NULL;

	if(response && response->len > 100)
	{
		if(response->str[0] == 0x25 &&
			response->str[1] == 0x50 &&
			response->str[2] == 0x44 &&
			response->str[3] == 0x46)
		{
			pdfData = g_malloc0(sizeof(*pdfData));
			pdfData->length = response->len;
			pdfData->data = (unsigned char*)response->str;
		}
		else
		{
			sci_log(LL_DEBUG, "%s: Got invalid pdf data", __func__);
		}
	}
	else
	{
		sci_log(LL_DEBUG, "%s: Return data to short to be a pdf at length %zu", __func__, response ? response->len : 0);
	}
	if(response)
		g_string_free(response, false);

	return pdfData;
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
	//ret = curl_easy_setopt(curlContext, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:106.0) Gecko/20100101 Firefox/106.0");
	//assert(ret == CURLE_OK);
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

GString* wpostUrl(const char* url, const char* data, int timeout)
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
	ret = curl_easy_setopt(curlContext, CURLOPT_POST, 1);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_POSTFIELDS, data);
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

GString* createJsonEntry(const int indent, const char* key, const char* value, bool quote, bool newline)
{
	if(!key || !value)
		return g_string_new("");

	GString* ret = g_string_new(NULL);

	for(int i = 0; i < indent; ++i)
		g_string_append_c(ret, '\t');

	g_string_append_c(ret, '\"');
	g_string_append(ret, key);
	g_string_append(ret, "\":");
	g_string_append_c(ret, ' ');
	if(quote)
		g_string_append_c(ret, '\"');
	char* escapedValue = g_strescape(value, NULL);
	g_string_append(ret, escapedValue);
	g_free(escapedValue);
	if(quote)
		g_string_append_c(ret, '\"');
	if(newline)
		g_string_append(ret, ",\n");

	return ret;
}
