/**
 * @file sci-conf.c
 * Configuration option handling for SCIPAPER
 * @author Carl Philipp Klemm <carl@uvos.xyz>
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
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "sci-conf.h"
#include "sci-log.h"

struct sci_conf_file
{
	gpointer keyfile;
	gchar *path;
	gchar *filename;
};

/** Pointer to the keyfile structure where config values are read from */
static struct sci_conf_file *conf_files = NULL;
static size_t sci_conf_file_count = 0;

static struct sci_conf_file *sci_conf_find_key_in_files(const gchar *group, const gchar *key)
{
	GError *error = NULL;
	
	if (conf_files) {
		for (size_t i = sci_conf_file_count; i > 0; --i) {
			if (g_key_file_has_key(conf_files[i-1].keyfile, group, key, &error) && 
				error == NULL)
			{
				g_clear_error(&error);
				return &(conf_files[i-1]);
			}
			else {
				g_clear_error(&error);
				error = NULL;
			}
		}
 	}
	return NULL;
}

static gpointer sci_conf_decide_keyfile_to_use(const gchar *group, const gchar *key, gpointer keyfile)
{
	if (keyfile == NULL)
	{
		struct sci_conf_file *conf_file;
		conf_file = sci_conf_find_key_in_files(group, key);
		if (conf_file == NULL)
			sci_log(LL_DEBUG, "sci-conf: Could not get config key %s/%s", group, key);
		else
			keyfile = conf_file->keyfile;
	}

	return keyfile;
}

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
			   const bool defaultval, gpointer keyfileptr)
{
	bool tmp = FALSE;
	GError *error = NULL;

	keyfileptr = sci_conf_decide_keyfile_to_use(group, key, keyfileptr);
	if (keyfileptr == NULL)
	{
		tmp = defaultval;
		goto EXIT;
	}

	tmp = g_key_file_get_boolean(keyfileptr, group, key, &error);

	if (error != NULL)
	{
		sci_log(LL_DEBUG, "sci-conf: "
			"Could not get config key %s/%s; %s; "
			"defaulting to `%d'",
			group, key, error->message, defaultval);
		tmp = defaultval;
	}

	g_clear_error(&error);

EXIT:
	return tmp;
}

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
		      const gint defaultval, gpointer keyfileptr)
{
	gint tmp = -1;
	GError *error = NULL;

	keyfileptr = sci_conf_decide_keyfile_to_use(group, key, keyfileptr);
	if (keyfileptr == NULL)
	{
		tmp = defaultval;
		goto EXIT;
	}

	tmp = g_key_file_get_integer(keyfileptr, group, key, &error);

	if (error != NULL)
	{
		sci_log(LL_DEBUG, "sci-conf: "
			"Could not get config key %s/%s; %s; "
			"defaulting to `%d'",
			group, key, error->message, defaultval);
		tmp = defaultval;
	}

	g_clear_error(&error);

EXIT:
	return tmp;
}

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
			    gsize *length, gpointer keyfileptr)
{
	gint *tmp = NULL;
	GError *error = NULL;

	keyfileptr = sci_conf_decide_keyfile_to_use(group, key, keyfileptr);
	if (keyfileptr == NULL)
	{
		*length = 0;
		goto EXIT;
	}

	tmp = g_key_file_get_integer_list(keyfileptr, group, key,
					  length, &error);

	if (error != NULL) {
		sci_log(LL_DEBUG, "sci-conf: "
			"Could not get config key %s/%s; %s",
			group, key, error->message);
		*length = 0;
	}

	g_clear_error(&error);

EXIT:
	return tmp;
}

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
			   const gchar *defaultval, gpointer keyfileptr)
{
	gchar *tmp = NULL;
	GError *error = NULL;
	
	keyfileptr = sci_conf_decide_keyfile_to_use(group, key, keyfileptr);
	if (keyfileptr == NULL)
	{
		if (defaultval != NULL)
			tmp = g_strdup(defaultval);
		goto EXIT;
	}

	tmp = g_key_file_get_string(keyfileptr, group, key, &error);

	if (error != NULL)
	{
		sci_log(LL_DEBUG, "sci-conf: "
			"Could not get config key %s/%s; %s; %s%s%s",
			group, key, error->message,
			defaultval ? "defaulting to `" : "no default set",
			defaultval ? defaultval : "",
			defaultval ? "'" : "");

		if (defaultval != NULL)
			tmp = g_strdup(defaultval);
	}

	g_clear_error(&error);

EXIT:
	return tmp;
}

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
				 gsize *length, gpointer keyfileptr)
{
	gchar **tmp = NULL;
	GError *error = NULL;

	keyfileptr = sci_conf_decide_keyfile_to_use(group, key, keyfileptr);
	if (keyfileptr == NULL)
	{
		*length = 0;
		goto EXIT;
	}

	tmp = g_key_file_get_string_list(keyfileptr, group, key,
					 length, &error);

	if (error != NULL)
	{
		sci_log(LL_DEBUG, "sci-conf: "
			"Could not get config key %s/%s; %s",
			group, key, error->message);
		*length = 0;
	}

	g_clear_error(&error);

EXIT:
	return tmp;
}

/**
 * Free configuration file
 *
 * @param keyfileptr A pointer to the keyfile to free
 */
void sci_conf_free_conf_file(gpointer keyfileptr)
{
	if (keyfileptr != NULL)
	{
		g_key_file_free(keyfileptr);
	}
}

/**
 * Read configuration file
 *
 * @param conffile The full path to the configuration file to read
 * @return A keyfile pointer on success, NULL on failure
 */
gpointer sci_conf_read_conf_file(const gchar *const conffile)
{
	GError *error = NULL;
	GKeyFile *keyfileptr;

	if ((keyfileptr = g_key_file_new()) == NULL)
		goto EXIT;

	if (g_key_file_load_from_file(keyfileptr, conffile,
				      G_KEY_FILE_NONE, &error) == FALSE)
	{
		sci_conf_free_conf_file(keyfileptr);
		keyfileptr = NULL;
		sci_log(LL_DEBUG, "sci-conf: Could not load %s; %s",
			conffile, error->message);
		goto EXIT;
	}

EXIT:
	g_clear_error(&error);

	return keyfileptr;
}

static bool sci_conf_is_ini_file(const char *filename)
{
	char *point_location = strrchr(filename, '.');
	if(point_location == NULL)
		return FALSE;
	else if(strcmp(point_location, ".ini") == 0)
		return TRUE;
	else
		return FALSE;
}

/**
 * Init function for the sci-conf component
 *
 * @return TRUE on success, FALSE on failure
 */
bool sci_conf_init(const char* fileName)
{
	sci_conf_file_count = fileName ? 2 : 1;

	conf_files = calloc(sci_conf_file_count, sizeof(*conf_files));
	
	conf_files[0].filename = g_strdup(G_STRINGIFY(SCI_SYSCONF_INI));
	conf_files[0].path     = g_strconcat(G_STRINGIFY(SCI_SYSCONF_DIR), "/", G_STRINGIFY(SCI_SYSCONF_INI), NULL);
	gpointer main_conf_file = sci_conf_read_conf_file(conf_files[0].path);
	if (main_conf_file == NULL)
	{
		sci_log(LL_ERR, "sci-conf: failed to open main config file %s %s",
				conf_files[0].path, g_strerror(errno));
		g_free(conf_files[0].filename);
		g_free(conf_files[0].path);
		free(conf_files);
		conf_files = NULL;
		return FALSE;
	}
	conf_files[0].keyfile = main_conf_file;

	if(fileName && sci_conf_is_ini_file(fileName))
	{
		conf_files[1].filename = g_strdup(fileName);
		conf_files[1].path     = g_strdup(fileName);
		gpointer application_conf_file = sci_conf_read_conf_file(conf_files[1].path);
		if (application_conf_file == NULL)
		{
			sci_log(LL_ERR, "sci-conf: failed to open main config file %s %s",
					conf_files[1].path, g_strerror(errno));
			g_free(conf_files[1].filename);
			g_free(conf_files[1].path);
			free(conf_files);
			conf_files = NULL;
			return FALSE;
		}
		conf_files[1].keyfile = application_conf_file;
	}
	else if(!sci_conf_is_ini_file(fileName))
	{
		sci_log(LL_ERR, "sci-conf: conf file %s is not an ini file!", fileName);
	}
	
	for (size_t i = 0; i < sci_conf_file_count; ++i)
		sci_log(LL_DEBUG, "sci-conf: using conf file %lu: %s", (unsigned long)i, conf_files[i].filename);

	return TRUE;
}

/**
 * Exit function for the sci-conf component
 */
void sci_conf_exit(void)
{
	for(size_t i = 0; i < sci_conf_file_count; ++i)
	{
		if (conf_files[i].filename)
			g_free(conf_files[i].filename);
		if (conf_files[i].path)
			g_free(conf_files[i].path);
		
		sci_conf_free_conf_file(conf_files[i].keyfile);
	}

	return;
}
