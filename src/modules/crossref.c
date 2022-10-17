/*
 * crossref.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 * 
 * crossref.c is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * crossref.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include "sci-modules.h"
#include "sci-log.h"
#include "sci-backend.h"
#include "types.h"
#include "scipaper.h"
#include "nxjson.h"
#include "utils.h"
#include "sci-conf.h"

/** Module name every module is required to have this*/
#define MODULE_NAME		"crossref"

/** Module information */
G_MODULE_EXPORT module_info_struct module_info = {
	/** Name of the module */
	.name = MODULE_NAME,
};

#define CROSSREF_URL_DOMAIN  "https://api.crossref.org/"
#define CROSSREF_URL_WORKS CROSSREF_URL_DOMAIN "works/"

struct CrPriv
{
	char* email;
	int rateLimitInterval;
	int id;
};

static DocumentMeta* cf_parse_work_json(char* jsonText, const DocumentMeta* metaIn)
{
	if(!jsonText)
		return NULL;

	const nx_json* json = nx_json_parse_utf8(jsonText);
	if(!json)
		return NULL;

	if(!g_str_equal(nx_json_get(json, "status")->text_value, "ok"))
	{
		sci_module_log(LL_WARN, "returned invalid status");
		return NULL;
	}
	if(!g_str_equal(nx_json_get(json, "message-type")->text_value, "work"))
	{
		sci_module_log(LL_WARN, "returned message of type %s instead of work", nx_json_get(json, "message-type")->text_value);
		return NULL;
	}

	const nx_json* entry = nx_json_get(json, "message");

	if(entry == NX_JSON_NULL)
	{
		sci_module_log(LL_WARN, "Message dosent contain document entry");
		return NULL;
	}

	DocumentMeta* meta = metaIn ? document_meta_copy(metaIn) : document_meta_new();

	meta->url    = g_strdup(nx_json_get(entry, "URL")->text_value);
	const nx_json* authorArray = nx_json_get(entry, "author");
	GString* authorString = g_string_new(NULL);
	for(size_t i = 0; i < authorArray->children.length; ++i)
	{
		const nx_json* author = nx_json_item(authorArray, i);
		g_string_append(authorString, nx_json_get(author, "given")->text_value);
		g_string_append(authorString, " ");
		g_string_append(authorString, nx_json_get(author, "family")->text_value);
		if(i < authorArray->children.length-1)
			g_string_append(authorString, ", ");
	}
	meta->author = g_strdup(authorString->str);
	g_string_free(authorString, true);

	meta->journal = g_strdup(nx_json_get(entry, "publisher")->text_value);
	meta->volume = g_strdup(nx_json_get(entry, "volume")->text_value);

	const nx_json* titleArray = nx_json_get(entry, "title");
	meta->title = g_strdup(nx_json_item(titleArray, 0)->text_value);

	nx_json_free(json);

	return meta;
}

static DocumentMeta** cf_fill_from_doi(size_t* count, const DocumentMeta* meta, struct CrPriv* priv)
{
	GString* url = g_string_new(CROSSREF_URL_WORKS);
	g_string_append(url, meta->doi);
	GString* jsonText = wgetUrl(url->str);
	g_string_free(url, true);

	DocumentMeta** ret = NULL;
	DocumentMeta* filledMeta = NULL;

	if(jsonText)
		filledMeta = cf_parse_work_json(jsonText->str, meta);

	if(filledMeta)
	{
		filledMeta->backendId = priv->id;
		*count = 1;
		DocumentMeta** ret = g_malloc0(sizeof(*ret));
		ret[0] = filledMeta;
	}

	if(jsonText)
		g_string_free(jsonText, true);

	return ret;
}

static DocumentMeta** cf_fill_meta_in(const DocumentMeta* meta, size_t* count, size_t maxCount, void* userData)
{
	struct CrPriv* priv = userData;
	(void)maxCount;

	if(meta->doi)
		return cf_fill_from_doi(count, meta, priv);

	return NULL;
}

G_MODULE_EXPORT const gchar *sci_module_init(void** data);
const gchar *sci_module_init(void** data)
{
	struct CrPriv* priv = g_malloc0(sizeof(*priv));
	priv->id = sci_plugin_register(MODULE_NAME, cf_fill_meta_in, NULL, NULL, priv);
	priv->rateLimitInterval = sci_conf_get_int("Crosref", "RateLimitInterval", 1, NULL);
	priv->email = sci_conf_get_string("Crosref", "Email", NULL, NULL);
	*data = priv;
	return NULL;
}

G_MODULE_EXPORT void sci_module_exit(void* data);
void sci_module_exit(void* data)
{
	struct CrPriv* priv = data;
	sci_plugin_unregister(priv->id);
	g_free(priv);
}
