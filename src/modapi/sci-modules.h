/**
 * @file sci-modules.h
 * Headers for the module handling for SCIPAPER
 * @author David Weinehall <david.weinehall@nokia.com>
 * @author Jonathan Wilson <jfwfreo@tpgi.com.au>
 * @author Carl Klemm <carl@uvos.xyz>
 *
 * scipaper is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * scipaper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with scipaper.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _SCI_MODULES_H_
#define _SCI_MODULES_H_

#include <glib.h>
#include <gmodule.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Name of Modules configuration group */
#define SCI_CONF_MODULES_GROUP		"Modules"

/** Name of configuration key for module path */
#define SCI_CONF_MODULES_PATH		"ModulePath"

/** Name of configuration key for general modules to load */
#define SCI_CONF_MODULES_MODULES	"Modules"

/** Default value for module path */
#define DEFAULT_SCI_MODULE_PATH		"/usr/lib/scipaper/modules"

/**
* @addtogroup MODAPI
* @{
*/

/**
 * Module information struct
 */
typedef struct {
	const gchar *const name; /**< Name of the plugin */
} module_info_struct;

/**
 * Modules must export a symbol of this type called sci_module_init, it will be called when the module is loaded
 *
 * @param data this pointer may be set by the module to pass context that is later given to sci_module_exit_fn()
 * @return If initalization is sucessfull this symbol shal return NULL, otherwise a const string describeing the problem is to be passed
 */
typedef const char* sci_module_init_fn(void** data);


/**
 * Modules must export a symbol of this type called sci_module_exit, it will be called just before the module is removed
 *
 * @param data this pointer gives this funciton context passed from sci_module_init_fn.
 */
typedef void sci_module_exit_fn(void* data);

/**@}*/

bool sci_modules_init(void);
void sci_modules_exit(void);

#ifdef __cplusplus
}
#endif


#endif /* _SCI_MODULES_H_ */
