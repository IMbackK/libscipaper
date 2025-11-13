/*
 * types.c
 * Copyright (C) Carl Philipp Klemm 2021 <carl@uvos.xyz>
 *
 * types.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * types.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "types.h"
#include <glib.h>
#include <stdio.h>
#include <assert.h>
#include "sci-log.h"
#include "utils.h"
#include "nxjson.h"

char* capability_flags_get_str(capability_flags_t capabilities)
{
	GString *string = g_string_new("");
	if(capabilities & SCI_CAP_FILL)
		g_string_append(string, "fill metadata, ");
	if(capabilities & SCI_CAP_GET_TEXT)
		g_string_append(string, "get full text, ");
	if(capabilities & SCI_CAP_GET_PDF)
		g_string_append(string, "get pdfs, ");

	g_string_truncate(string, string->len-2);

	char* str = string->str;
	g_string_free(string, false);
	return str;
}

RequestReturn* request_return_new(size_t count, size_t maxCount)
{
	RequestReturn* ret = g_malloc0(sizeof(*ret));
	ret->documents = g_malloc0(sizeof(*ret->documents)*count);
	ret->count = count;
	ret->maxCount = maxCount;
	return ret;
}

void request_return_free(RequestReturn* reqRet)
{
	if(!reqRet)
		return;

	document_meta_free_list(reqRet->documents, reqRet->count);
	g_free(reqRet);
}

DocumentMeta* document_meta_new(void)
{
	DocumentMeta* meta = g_malloc0(sizeof(DocumentMeta));
	meta->references = -1;
	return meta;
}

DocumentMeta* document_meta_copy(const DocumentMeta* meta)
{
	DocumentMeta* copy = g_malloc0(sizeof(*copy));

	copy->doi = g_strdup(meta->doi);
	copy->url = g_strdup(meta->url);
	copy->year = meta->year;
	copy->publisher = g_strdup(meta->publisher);
	copy->volume = g_strdup(meta->volume);
	copy->pages = g_strdup(meta->pages);
	copy->author = g_strdup(meta->author);
	copy->title = g_strdup(meta->title);
	copy->journal = g_strdup(meta->journal);
	copy->issn = g_strdup(meta->issn);
	copy->keywords = g_strdup(meta->keywords);
	copy->downloadUrl = g_strdup(meta->downloadUrl);
	copy->abstract = g_strdup(meta->abstract);
	copy->references = meta->references;
	copy->searchText = g_strdup(meta->searchText);
	copy->backendId = meta->backendId;
	copy->hasFullText = meta->hasFullText;
	if(meta->backendData)
	{
		assert(meta->backend_data_copy_fn);
		copy->backendData = meta->backend_data_copy_fn(meta->backendData);
	}
	copy->backend_data_free_fn = meta->backend_data_free_fn;
	copy->backend_data_copy_fn = meta->backend_data_copy_fn;
	copy->compleatedLookup = meta->compleatedLookup;

	return copy;
}

void document_meta_free(DocumentMeta* meta)
{
	if(meta)
	{
		g_free(meta->doi);
		g_free(meta->url);
		g_free(meta->publisher);
		g_free(meta->volume);
		g_free(meta->pages);
		g_free(meta->author);
		g_free(meta->title);
		g_free(meta->journal);
		g_free(meta->issn);
		g_free(meta->keywords);
		g_free(meta->downloadUrl);
		g_free(meta->abstract);

		if(meta->backendData)
		{
			assert(meta->backend_data_free_fn);
			meta->backend_data_free_fn(meta->backendData);
		}

		g_free(meta->searchText);
		g_free(meta);
	}
}

char* document_meta_get_string(const DocumentMeta* meta)
{
	return g_strdup_printf("Document:\nDOI: %s\nTitle: %s\nAuthor: %s\nJournal: %s\nKeywords: %s\nAbstract: %s\n",
		meta->doi ? meta->doi : "", meta->title ? meta->title : "", meta->author ? meta->author : "",
		meta->journal ? meta->journal : "", meta->keywords ? meta->keywords : "",
		meta->abstract ? meta->abstract : "");
}

void document_meta_free_list(DocumentMeta** meta, size_t length)
{
	if(meta)
	{
		for(size_t i = 0; i < length; ++i)
			document_meta_free(meta[i]);
		g_free(meta);
	}
}

void document_meta_combine(DocumentMeta* target, const DocumentMeta* source)
{
	if(!target || !source)
		return;
	if(!target->doi)
		target->doi = g_strdup(source->doi);
	if(!target->url)
		target->url = g_strdup(source->url);
	if(!target->year)
		target->year = source->year;
	if(!target->publisher)
		target->publisher = g_strdup(source->publisher);
	if(!target->volume)
		target->volume = g_strdup(source->volume);
	if(!target->pages)
		target->pages = g_strdup(source->pages);
	if(!target->author)
		target->author = g_strdup(source->author);
	if(!target->journal)
		target->journal = g_strdup(source->journal);
	if(!target->issn)
		target->issn = g_strdup(source->issn);
	if(!target->keywords)
		target->keywords =  g_strdup(source->keywords);
	if(!target->downloadUrl)
		target->downloadUrl =  g_strdup(source->downloadUrl);
	if(!target->abstract)
		target->abstract =  g_strdup(source->abstract);
	if(target->references < source->references)
		target->references = source->references;
}

char* document_meta_get_json_only_fillrq(const DocumentMeta* meta, const FillReqest rq, const char* fullText, size_t* length)
{
	if(length)
		*length = 0;
	if(!meta)
		return NULL;

	GString* string = g_string_new("{\n");
	GString* entry;

	if(rq.doi)
	{
		entry = createJsonEntry(1, "doi", meta->doi, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.url)
	{
		entry = createJsonEntry(1, "url", meta->url, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.year && meta->year != 0)
	{
		char* yearStr = g_strdup_printf("%lu", meta->year);
		entry = createJsonEntry(1, "year", yearStr, false, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
		g_free(yearStr);
	}

	if(rq.publisher)
	{
		entry = createJsonEntry(1, "publisher", meta->publisher, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.volume)
	{
		entry = createJsonEntry(1, "volume", meta->volume, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.pages)
	{
		entry = createJsonEntry(1, "pages", meta->pages, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.author)
	{
		entry = createJsonEntry(1, "author", meta->author, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.title)
	{
		entry = createJsonEntry(1, "title", meta->title, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.journal)
	{
		entry = createJsonEntry(1, "journal", meta->journal, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.issn)
	{
		entry = createJsonEntry(1, "issn", meta->issn, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.keywords)
	{
		entry = createJsonEntry(1, "keywords", meta->keywords, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.references && meta->references >= 0)
	{
		char* referanceStr = g_strdup_printf("%i", meta->references);
		entry = createJsonEntry(1, "referances", referanceStr, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
		g_free(referanceStr);
	}

	if(rq.downloadUrl)
	{
		entry = createJsonEntry(1, "download-url", meta->downloadUrl, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	if(rq.abstract)
	{
		entry = createJsonEntry(1, "abstract", meta->abstract, true, true);
		g_string_append(string, entry->str);
		g_string_free(entry, true);
	}

	entry = createJsonEntry(1, "full-text", fullText, true, true);
	g_string_append(string, entry->str);
	g_string_free(entry, true);

	g_string_append(string, "}\n");

	if(length)
		*length = string->len;
	return string->str;
}

char* document_meta_get_json(const DocumentMeta* meta, const char* fullText, size_t* length)
{
	FillReqest rq;
	memset(&rq, 0xFF, sizeof(FillReqest));
	return document_meta_get_json_only_fillrq(meta, rq, fullText, length);
}

DocumentMeta* document_meta_load_from_json(char* jsonFile)
{
	const nx_json* json = nx_json_parse_utf8(jsonFile);
	if(!json)
	{
		sci_log(LL_ERR, "%s: Could not load json entry in given file", __func__);
		return NULL;
	}

	DocumentMeta* meta = document_meta_new();

	meta->doi = g_strdup(nx_json_get(json, "doi")->text_value);
	meta->url = g_strdup(nx_json_get(json, "url")->text_value);
	meta->year = nx_json_get(json, "year")->int_value;
	meta->references = nx_json_get(json, "references")->int_value;
	meta->publisher = g_strdup(nx_json_get(json, "publisher")->text_value);
	meta->volume = g_strdup(nx_json_get(json, "volume")->text_value);
	meta->pages = g_strdup(nx_json_get(json, "pages")->text_value);
	meta->author = g_strdup(nx_json_get(json, "author")->text_value);
	meta->title = g_strdup(nx_json_get(json, "title")->text_value);
	meta->journal = g_strdup(nx_json_get(json, "journal")->text_value);
	meta->issn = g_strdup(nx_json_get(json, "issn")->text_value);
	meta->keywords = g_strdup(nx_json_get(json, "keywords")->text_value);
	meta->downloadUrl = g_strdup(nx_json_get(json, "download-url")->text_value);
	meta->abstract = g_strdup(nx_json_get(json, "abstract")->text_value);

	nx_json_free(json);

	return meta;
}


DocumentMeta* document_meta_load_from_json_file(const char* jsonFileName)
{
	char* text;
	GError *error = NULL;

	bool ret = g_file_get_contents(jsonFileName, &text, NULL, &error);
	if(!ret)
	{
		sci_log(LL_ERR, "%s: %s", __func__, error->message);
		g_error_free(error);
		return NULL;
	}

	DocumentMeta* meta = document_meta_load_from_json(text);
	g_free(text);
	return meta;
}

char* document_meta_load_full_text_from_json_file(const char* jsonFileName)
{
	char* jsonFile;
	GError *error = NULL;

	bool ret = g_file_get_contents(jsonFileName, &jsonFile, NULL, &error);
	if(!ret)
	{
		sci_log(LL_ERR, "%s: %s", __func__, error->message);
		g_error_free(error);
		return NULL;
	}
	const nx_json* json = nx_json_parse_utf8(jsonFile);
	if(!json)
	{
		sci_log(LL_ERR, "%s: Could not load json entry in given file", __func__);
		return NULL;
	}
	return g_strdup(nx_json_get(json, "full-text")->text_value);
}

char* document_meta_get_biblatex(const DocumentMeta* meta, size_t* length, const char* type)
{
	if(!type)
		type = "article";

	if(length)
		*length = 0;

	if(!meta->author)
	{
		sci_log(LL_DEBUG, "%s: the document meta must contain at least an author field", __func__);
		return NULL;
	}
	GString* string = g_string_new("@");
	g_string_append(string, type);
	g_string_append(string, "{");

	char** authorTokens = g_str_tokenize_and_fold(meta->author, NULL, NULL);
	GString* authorString = g_string_new(NULL);
	for(int i = 0; authorTokens[i]; ++i)
	{
		if(i == 0)
			g_string_append(authorString, authorTokens[i]);
		else
			g_string_append_c(authorString, authorTokens[i][0]);

		g_free(authorTokens[i]);
	}
	g_free(authorTokens);
	g_string_ascii_up(authorString);
	if(meta->year)
		g_string_append_printf(authorString, "%lu", meta->year);
	else
		g_string_append_printf(authorString, "%u", (unsigned int)(g_random_int_range(0, (1 << 16))));
	g_string_append(string, authorString->str);
	g_string_append(string, ",\n");
	g_string_free(authorString, true);

	authorString = g_string_new(meta->author);
	g_string_replace(authorString, ", ", " and ", 0);

	g_string_append_printf(string, "\tauthor={%s},\n", authorString->str);
	g_string_free(authorString, true);
	if(meta->title)
		g_string_append_printf(string, "\ttitle={%s},\n", meta->title);
	if(meta->doi)
		g_string_append_printf(string, "\tdoi={%s},\n", meta->doi);
	if(meta->url)
		g_string_append_printf(string, "\turl={%s},\n", meta->url);
	if(meta->year)
		g_string_append_printf(string, "\tyear={%lu},\n", meta->year);
	if(meta->publisher)
		g_string_append_printf(string, "\tpublisher={%s},\n", meta->publisher);
	if(meta->volume)
		g_string_append_printf(string, "\tvolume={%s},\n", meta->volume);
	if(meta->pages)
		g_string_append_printf(string, "\tpages={%s},\n", meta->pages);
	if(meta->issn)
		g_string_append_printf(string, "\tissn={%s},\n", meta->issn);
	if(meta->keywords)
		g_string_append_printf(string, "\tkeywords={%s},\n", meta->keywords);
	if(meta->journal)
		g_string_append_printf(string, "\tjournal={%s},\n", meta->journal);
	g_string_append_c(string, '}');
	g_string_append_c(string, '\n');

	if(length)
		*length = string->len;
	char* str = string->str;
	g_string_free(string, false);
	return str;
}

bool document_meta_save_only_fillrq(const char* fileName, const DocumentMeta* meta, const FillReqest fq, const char* fullText)
{
	size_t length;
	char* jsonText = document_meta_get_json_only_fillrq(meta, fq, fullText, &length);

	if(jsonText)
	{
		GError *error = NULL;
		bool ret = g_file_set_contents_full(fileName, (gchar*)jsonText, length, G_FILE_SET_CONTENTS_NONE, 0666, &error);
		g_free(jsonText);
		if(!ret)
		{
			sci_log(LL_ERR, "%s: %s", __func__, error->message);
			g_error_free(error);
			return false;
		}
		return true;
	}
	return false;
}

bool document_meta_save(const char* fileName, const DocumentMeta* meta, const char* fullText)
{
	FillReqest rq;
	memset(&rq, 0xFF, sizeof(FillReqest));
	return document_meta_save_only_fillrq(fileName, meta, rq, fullText);
}

bool document_meta_is_equal(const DocumentMeta* a, const DocumentMeta* b)
{
	if(a == b)
		return true;
	if(!a || !b)
		return false;

	if(g_strcmp0(a->doi, b->doi) != 0)
		return false;
	if(g_strcmp0(a->url, b->url) != 0)
		return false;
	if(a->year != b->year)
		return false;
	if(g_strcmp0(a->publisher, b->publisher) != 0)
		return false;
	if(g_strcmp0(a->volume, b->volume) != 0)
		return false;
	if(g_strcmp0(a->pages, b->pages) != 0)
		return false;
	if(g_strcmp0(a->author, b->author) != 0)
		return false;
	if(g_strcmp0(a->title, b->title) != 0)
		return false;
	if(g_strcmp0(a->journal, b->journal) != 0)
		return false;
	if(g_strcmp0(a->issn, b->issn) != 0)
		return false;
	if(g_strcmp0(a->keywords, b->keywords) != 0)
		return false;

	return true;
}

void pdf_data_free(PdfData* data)
{
	document_meta_free(data->meta);
	g_free(data->data);
	g_free(data);
}
