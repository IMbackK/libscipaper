/**
 * @file sci-modules.c
 * module handling for sci
 * @author David Weinehall <david.weinehall@nokia.com>
 * @author Jonathan Wilson <jfwfreo@tpgi.com.au>
 * @author Carl Klemm <carl@uvos.xyz>
 *
 * sci is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * sci is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with sci.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <glib.h>
#include "sci-modules.h"
#include "sci-log.h"
#include "sci-conf.h"

struct sci_module {
	GModule *module;
	void *data;
};

/** List of all loaded modules */
static GSList *modules = NULL;

static BackendInfo *sci_modules_get_info(struct sci_module *module)
{
	gpointer mip = NULL;
	if(g_module_symbol(module->module, "backend_info", (void**)&mip) == FALSE)
		return NULL;
	return (BackendInfo*)mip;
}

static bool sci_modules_init_modules(void)
{
	for (GSList *element = modules; element; element = element->next)
	{
		gpointer fnp = NULL;
		sci_module_init_fn *init_fn;
		struct sci_module *module = element->data;
		if(g_module_symbol(module->module, "sci_module_init", (void**)&fnp) == FALSE)
		{
			sci_log(LL_ERR, "faled to load module %s: missing symbol sci_module_init",
					   sci_modules_get_info(module)->name);
			return FALSE;
		}
		init_fn = (sci_module_init_fn*)fnp;
		const char* result = init_fn(&module->data);
		if(result)
		{
			sci_log(LL_ERR, "faled to load module %s: %s",
					   sci_modules_get_info(module)->name, result);
			return FALSE;
		}
	}

	return TRUE;
}

static void sci_modules_load(gchar **modlist)
{
	gchar *path = NULL;
	int i;

	path = sci_conf_get_string(SCI_CONF_MODULES_GROUP,
				   SCI_CONF_MODULES_PATH,
				   DEFAULT_SCI_MODULE_PATH,
				   NULL);

	for (i = 0; modlist[i]; i++)
	{
		struct sci_module *module = g_malloc(sizeof(*module));
		gchar *tmp = g_module_build_path(path, modlist[i]);

		sci_log(LL_DEBUG, "Loading module: %s from %s", modlist[i], path);

		if ((module->module = g_module_open(tmp, 0)) != NULL) {
			BackendInfo *info = sci_modules_get_info(module);
			bool blockLoad = FALSE;

			if(!info)
			{
				sci_log(LL_ERR, "Failed to retrieve module information for: %s did you forget to export a backend_info symbol?", modlist[i]);
				g_module_close(module->module);
				g_free(module);
				blockLoad = TRUE;
			}

			if (!blockLoad)
				modules = g_slist_append(modules, module);
		}
		else
		{
			sci_log(LL_WARN, "Failed to load module %s: %s; skipping", modlist[i], g_module_error());
		}

		g_free(tmp);
	}

	g_free(path);
}

/**
 * Init function for the sci-modules component
 *
 * @return TRUE on success, FALSE on failure
 */
bool sci_modules_init(void)
{
	gchar **modlist = NULL;
	gsize length;

	/* Get the list modules to load */
	modlist = sci_conf_get_string_list(SCI_CONF_MODULES_GROUP,
					   SCI_CONF_MODULES_MODULES,
					   &length,
					   NULL);

	if(modlist)
	{
		sci_modules_load(modlist);
		g_strfreev(modlist);

		return sci_modules_init_modules();
	}

	return TRUE;
}

/**
 * Exit function for the sci-modules component
 */
void sci_modules_exit(void)
{
	struct sci_module *module;
	gint i;

	if(modules != NULL)
	{
		for(i = 0; (module = g_slist_nth_data(modules, i)) != NULL; i++) {
			gpointer fnp = NULL;

			if(g_module_symbol(module->module, "sci_module_exit", (void**)&fnp) == TRUE)
			{
				sci_module_exit_fn *exit_fn = (sci_module_exit_fn*)fnp;
				exit_fn(module->data);
			}
			else
			{
				sci_log(LL_DEBUG, "module %s: has no sci_module_exit", sci_modules_get_info(module)->name);
			}
			
			g_module_close(module->module);
			g_free(module);
		}

		g_slist_free(modules);
		modules = NULL;
	}
}
