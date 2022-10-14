/**
 * @file sci-conf.h
 * Headers for the configuration option handling for SPHONE
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
#ifndef _SPHONE_CONF_H_
#define _SPHONE_CONF_H_

#include <glib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @addtogroup MODAPI
* @{
*/

/**@{*/

/**
 * Get a boolean configuration value
 *
 * @param group The configuration group to get the value from
 * @param key The configuration key to get the value of
 * @param defaultval The default value to use if the key isn't set
 * @param keyfileptr A keyfile pointer, or NULL to use the default keyfile
 * @return The configuration value on success, the default value on failure
 */
bool sci_conf_get_bool(const gchar *group, const gchar *key,
			   const bool defaultval, gpointer keyfileptr);

/**
 * Get an integer configuration value
 *
 * @param group The configuration group to get the value from
 * @param key The configuration key to get the value of
 * @param defaultval The default value to use if the key isn't set
 * @param keyfileptr A keyfile pointer, or NULL to use the default keyfile
 * @return The configuration value on success, the default value on failure
 */
gint sci_conf_get_int(const gchar *group, const gchar *key,
		      const gint defaultval, gpointer keyfileptr);

/**
 * Get an integer list configuration value
 *
 * @param group The configuration group to get the value from
 * @param key The configuration key to get the value of
 * @param length The length of the list
 * @param keyfileptr A keyfile pointer, or NULL to use the default keyfile
 * @return The configuration value on success, NULL on failure
 */
gint *sci_conf_get_int_list(const gchar *group, const gchar *key,
			    gsize *length, gpointer keyfileptr);

/**
 * Get a string configuration value
 *
 * @param group The configuration group to get the value from
 * @param key The configuration key to get the value of
 * @param defaultval The default value to use if the key isn't set
 * @param keyfileptr A keyfile pointer, or NULL to use the default keyfile
 * @return The configuration value on success, the default value on failure
 */
gchar *sci_conf_get_string(const gchar *group, const gchar *key,
			   const gchar *defaultval, gpointer keyfileptr);

/**
 * Get a string list configuration value
 *
 * @param group The configuration group to get the value from
 * @param key The configuration key to get the value of
 * @param length The length of the list
 * @param keyfileptr A keyfile pointer, or NULL to use the default keyfile
 * @return The configuration value on success, NULL on failure
 */
gchar **sci_conf_get_string_list(const gchar *group, const gchar *key,
				 gsize *length, gpointer keyfileptr);
/**@}*/
/**@}*/

gpointer sci_conf_read_conf_file(const gchar *const conffile);
void sci_conf_free_conf_file(gpointer keyfileptr);

bool sci_conf_init(const char* fileName);
void sci_conf_exit(void);

#ifdef __cplusplus
}
#endif
#endif /* _SPHONE_CONF_H_ */
