/**
 * @file sci-log.h
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
#pragma once

#include <syslog.h>	/* LOG_DAEMON, LOG_USER */

#ifdef __cplusplus
extern "C" {
#endif

#define SCI_LOG_SYSLOG			1	/**< Log to syslog */
#define SCI_LOG_STDERR			0	/**< Log to stderr */

/**
* @addtogroup MODAPI
* @{
*/

/**
 * Print a string to the log
 *
 * @param loglevel Log level to use
 * @param fmt printf style format string
 * @param ... Values to place into format string
 */
#define sci_module_log(loglevel, fmt, ...) sci_log(loglevel, "%s: " fmt, MODULE_NAME, ##__VA_ARGS__)

/**@}*/

/** Severity of loglevels */
typedef enum {
	LL_NONE = 0,			/**< No logging at all */
	LL_CRIT = 1,			/**< Critical error */
	LL_ERR = 2,			/**< Error */
	LL_WARN = 3,			/**< Warning */
	LL_DEFAULT = LL_WARN,		/**< Default log level */
	LL_INFO = 4,			/**< Informational message */
	LL_DEBUG = 5			/**< Useful when debugging */
} loglevel_t;

void sci_log(const loglevel_t loglevel, const char *const fmt, ...)
	__attribute__((format(printf, 2, 3)));
void sci_log_set_verbosity(const int verbosity);
void sci_log_open(const char *const name, const int facility, const int type);
void sci_log_close(void);

#ifdef __cplusplus
}
#endif
