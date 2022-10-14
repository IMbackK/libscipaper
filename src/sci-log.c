/**
 * @file sci-log.c
 * Logging functions
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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif /* _BSD_SOURCE */
#include <syslog.h>
#include "sci-log.h"

static unsigned int logverbosity = LL_WARN;	/**< Log verbosity */
static int logtype = SCI_LOG_SYSLOG;		/**< Output for log messages */
static char *logname = NULL;

/**
 *
 * @param loglevel The level of severity for this message
 * @param fmt The format string for this message
 * @param ... Input to the format string
 */
void sci_log(const loglevel_t loglevel, const char *const fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	if (logverbosity >= loglevel) {
		if (logtype == SCI_LOG_STDERR) {
			fprintf(stderr, "%s: ", logname);
			vfprintf(stderr, fmt, args);
			size_t len = strlen(fmt);
			if(len == 0 || fmt[strlen(fmt)-1] != '\n')
				fprintf(stderr, "\n");
		} else {
			switch (loglevel) {
				case LL_DEBUG:
					vsyslog(LOG_DEBUG, fmt, args);
					break;

				case LL_ERR:
					vsyslog(LOG_ERR, fmt, args);
					break;

				case LL_CRIT:
					vsyslog(LOG_CRIT, fmt, args);
					break;

				case LL_INFO:
					vsyslog(LOG_INFO, fmt, args);
					break;

				case LL_WARN:
				default:
					vsyslog(LOG_WARNING, fmt, args);
													break;
			}
		}
	}

	va_end(args);
}

/**
 * Set log verbosity
 * messages with loglevel higher than or equal to verbosity will be logged
 *
 * @param verbosity minimum level for log level
 */
void sci_log_set_verbosity(const int verbosity)
{
	logverbosity = verbosity;
}

/**
 * Open log
 *
 * @param name identifier to use for log messages
 * @param facility the log facility; normally LOG_USER or LOG_DAEMON
 * @param type log type to use; SCI_LOG_STDERR or SCI_LOG_SYSLOG
 */
void sci_log_open(const char *const name, const int facility, const int type)
{
	logtype = type;

	if (logtype == SCI_LOG_SYSLOG)
		openlog(name, LOG_PID | LOG_NDELAY, facility);
	else
		logname = strdup(name);
}

/**
 * Close log
 */
void sci_log_close(void)
{
	if (logname)
		free(logname);

	if (logtype == SCI_LOG_SYSLOG)
		closelog();
}
