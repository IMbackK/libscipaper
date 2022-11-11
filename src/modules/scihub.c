/*
 * scihub.c
 * Copyright (C) Carl Philipp Klemm 2022 <carl@uvos.xyz>
 *
 * scihub.c is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * scihub.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <libxml/xmlmemory.h>
#include <libxml/HTMLparser.h>
#include "sci-modules.h"
#include "sci-log.h"
#include "sci-backend.h"
#include "sci-conf.h"
#include "types.h"
#include "scipaper.h"
#include "utils.h"

/** Module name every module is required to have this*/
#define MODULE_NAME		"scihub"

/** Module information */
G_MODULE_EXPORT BackendInfo backend_info = {
	/** Name of the module */
	.name = MODULE_NAME,
	.capabilities = SCI_CAP_GET_PDF,
};

struct ScihubPriv {
	char* baseUrl;
	int id;
	int timeout;
};

static char* get_pdf_url(xmlNode* node)
{
	xmlNode *curNode = NULL;

	char* result = NULL;

	for(curNode = node; curNode && !result; curNode = curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			xmlChar* prop = xmlGetProp(curNode, (xmlChar*)"onclick");
			if(prop && g_strstr_len((char*)prop, -1 ,"pdf"))
			{
				sci_module_log(LL_DEBUG, "prop: %s\n", (char*)prop);
				char* urlStart = g_strstr_len((char*)prop, -1 ,"=");
				if(!urlStart || urlStart[1] == '\0')
				{
					xmlFree(prop);
					return NULL;
				}

				bool quotes = urlStart[1] == '\'' && urlStart[2] != '\0';

				char* url = g_strdup(urlStart+1+quotes);
				url[strlen(url)-quotes] = '\0';
				sci_module_log(LL_DEBUG, "url: %s\n", url);
				xmlFree(prop);
				return url;
			}
		}

		result = get_pdf_url(curNode->children);
	}

	return result;
}

static char* get_pdf_url_simple(GString* htmlText)
{
	sci_module_log(LL_DEBUG, "test: %s\n", htmlText->str);
	char* dlLoc = g_strstr_len(htmlText->str, -1 ,"download=true");

	if(!dlLoc)
	{
		sci_module_log(LL_DEBUG, "No download\n");
		return NULL;
	}

	char* c = dlLoc;
	for(; c > htmlText->str && *c && *c != '<' && *c != '>'; --c)
	{
		if(*c == '\'')
			break;
	}

	if(c <= htmlText->str || *c != '\'')
		return NULL;

	char* begin = c+1;
	c = g_strstr_len(begin, -1 ,"'");
	c = '\0';
	sci_module_log(LL_DEBUG, "url: %s\n", begin);
	return g_strdup(begin);
}

static PdfData* scihub_get_document_pdf_data(const DocumentMeta* meta, void* userData)
{
	sci_module_log(LL_DEBUG, "%s", __func__);
	struct ScihubPriv* priv = userData;

	if(!meta->doi)
	{
		sci_module_log(LL_DEBUG, "scihub works on dois only");
		return NULL;
	}

	GString* url = g_string_new(priv->baseUrl);
	g_string_append(url, meta->doi);

	sci_module_log(LL_WARN, "Geting scihub page from %s", url->str);
	GString* htmlText = wgetUrl(url->str, priv->timeout);
	g_string_free(url, true);

	xmlDocPtr htmlPage = htmlReadMemory(htmlText->str, htmlText->len, "/" ,NULL, XML_PARSE_RECOVER);
	if(!htmlPage)
	{
		sci_module_log(LL_WARN, "Got invalid scihup page");
		g_string_free(htmlText, true);
		return NULL;
	}

	char* pdfUrl = get_pdf_url(xmlDocGetRootElement(htmlPage));

	PdfData* pdfData = NULL;
	if(!pdfUrl)
		pdfUrl = get_pdf_url_simple(htmlText);

	if(pdfUrl)
	{
		pdfData = wgetPdf((char*)pdfUrl, priv->timeout);
		g_free(pdfUrl);
	}
	else
	{
		sci_module_log(LL_WARN, "Could not get pdf url from scihub page");
	}

	if(!pdfData)
		sci_module_log(LL_WARN, "Unable to grab pdf from scihub pdf link");

	g_string_free(htmlText, true);
	xmlFreeDoc(htmlPage);

	return pdfData;
}


G_MODULE_EXPORT const gchar *sci_module_init(void** data);
const gchar *sci_module_init(void** data)
{
	struct ScihubPriv* priv = g_malloc0(sizeof(*priv));
	*data = priv;

	priv->timeout = sci_conf_get_int("Scihub", "Timeout", 20, NULL);
	priv->baseUrl = sci_conf_get_string("Scihub", "Url", NULL, NULL);
	if(!priv->baseUrl)
		return "A Scihub url is required in conf";

	sci_module_log(LL_DEBUG, "scihub register");
	priv->id = sci_plugin_register(&backend_info, NULL, NULL, scihub_get_document_pdf_data, priv);

	return NULL;
}

G_MODULE_EXPORT void sci_module_exit(void* data);
void sci_module_exit(void* data)
{
	struct ScihubPriv* priv = data;
	sci_plugin_unregister(priv->id);
	xmlCleanupParser();

	g_free(priv->baseUrl);
	g_free(priv);
}
