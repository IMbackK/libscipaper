/*
 * scipaper.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 *
 * scipaper.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * scipaper.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "scipaper.h"

#include <stdbool.h>
#include <assert.h>
#include <glib.h>

#include "sci-log.h"
#include "sci-conf.h"
#include "sci-modules.h"
#include "scipaper.h"

static const VersionFixed version = {1, 0, 0};

bool sci_paper_init(const char* config_file, const char* data, size_t length)
{
	sci_log_open("libscipaper", LOG_USER, SCI_LOG_STDERR);

	if(!sci_conf_init(config_file, data, length))
		return false;

	if(!sci_modules_init())
		return false;

	return true;
}

void sci_paper_exit(void)
{
	sci_modules_exit();
	sci_conf_exit();
	size_t backendCount = sci_get_backend_count();
	if(backendCount != 0)
		sci_log(LL_WARN, "%zu backend(s) have failed to unregister!!!", backendCount);
}

DocumentMeta* sci_find_by_doi(const char* doi, int backendId)
{
	DocumentMeta meta = {0};
	meta.doi = (char*)doi;
	meta.backendId = backendId;
	RequestReturn* documents = sci_fill_meta(&meta, NULL, 1, 0);
	if(documents)
	{
		DocumentMeta* meta = document_meta_copy(documents->documents[0]);
		request_return_free(documents);
		return meta;
	}
	else
	{
		return NULL;
	}
}

RequestReturn* sci_find_by_author(const char* author, size_t maxCount)
{
	DocumentMeta meta = {0};
	meta.author = (char*)author;
	return sci_fill_meta(&meta, NULL, maxCount, 0);
}

DocumentMeta* sci_find_by_title(const char* title)
{
	DocumentMeta meta = {0};
	meta.title = (char*)title;
	RequestReturn* documents = sci_fill_meta(&meta, NULL, 1, 0);
	if(documents)
	{
		DocumentMeta* meta = document_meta_copy(documents->documents[0]);
		request_return_free(documents);
		return meta;
	}
	else
	{
		return NULL;
	}
}

RequestReturn* sci_find_by_journal(const char* jornal, size_t maxCount)
{
	DocumentMeta meta = {0};
	meta.journal = (char*)jornal;
	return sci_fill_meta(&meta, NULL, maxCount, 0);
}

bool sci_save_pdf_to_file(const PdfData* data, const char* fileName)
{
	if(!data || !data->data)
		return false;

	GError *error = NULL;
	if(!g_file_set_contents_full(fileName, (gchar*)data->data, data->length, G_FILE_SET_CONTENTS_NONE, 0666, &error))
	{
		sci_log(LL_ERR, "%s: %s", __func__, error->message);
		g_error_free(error);
		return false;
	}

	return true;
}

bool sci_save_document_to_file(const DocumentMeta* meta, const char* fileName)
{
	PdfData* data = sci_get_document_pdf_data(meta);
	if(!data)
		return false;
	bool ret = sci_save_pdf_to_file(data, fileName);
	pdf_data_free(data);
	return ret;
}

const VersionFixed* sci_get_version(void)
{
	return &version;
}
