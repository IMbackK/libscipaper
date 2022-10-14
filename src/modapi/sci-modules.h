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

typedef struct {
	const gchar *const name;
} module_info_struct;

bool sci_modules_init(void);
void sci_modules_exit(void);

typedef const char* sci_module_init_fn(void** data);
typedef void sci_module_exit_fn(void* data);

#ifdef __cplusplus
}
#endif


#endif /* _SCI_MODULES_H_ */
