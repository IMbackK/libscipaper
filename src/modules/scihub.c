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
#include <libxml/parser.h>
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
};

struct ScihubPriv {
	char* baseUrl;
	int id;
	int timeout;
};

static void
print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            sci_module_log(LL_DEBUG, "node type: Element, name: %s\n", cur_node->name);
        }

        print_element_names(cur_node->children);
    }
}

static GString* get_pdf_url(xmlDocPtr page)
{
	xmlNode *rootNode = xmlDocGetRootElement(page);
	print_element_names(rootNode);
	return NULL;
}

static PdfData* scihub_get_document_pdf_data(const DocumentMeta* meta, void* userData)
{
	struct ScihubPriv* priv = userData;

	if(!meta->doi)
		return NULL;

	GString* url = g_string_new(priv->baseUrl);
	g_string_append(url, meta->doi);

	GString* htmlText = wgetUrl(url->str, priv->timeout);
	g_string_free(url, true);

	xmlDocPtr htmlPage = xmlReadMemory(htmlText->str, htmlText->len, "/" ,NULL, XML_PARSE_RECOVER);
	if(!htmlPage)
	{
		sci_module_log(LL_WARN, "Got invalid scihup page");
		g_string_free(htmlText, true);
		return NULL;
	}

	GString* pdfUrl = get_pdf_url(htmlPage);

	PdfData* pdfData = NULL;
	if(pdfUrl)
		pdfData = wgetPdf(pdfUrl->str, priv->timeout);
	else
		sci_module_log(LL_WARN, "Could not get pdf url from scihub page");

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
