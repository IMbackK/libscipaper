#include "scipaper.h"

#include <stdbool.h>

#include "sci-log.h"
#include "sci-conf.h"
#include "scipaper.h"

bool sci_paper_init(const char* config_file)
{
	sci_log_open("libscipaper", LOG_USER, SCI_LOG_STDERR);

	if(!sci_conf_init(config_file))
		return false;

	if(!sci_modules_init())
		return false;
}

void sci_paper_exit(void)
{
	sci_modules_exit();
	sci_conf_exit();
}

DocumentMeta* sci_find_by_doi(const char* doi)
{
	DocumentMeta meta = {0};
	meta.doi = doi;
	size_t count;
	DocumentMeta** documentMetas = sci_fill_meta(&meta, &count, 1);
	if(count > 0)
		return documentMetas[0]
	else
		return NULL;
}

DocumentMeta** sci_find_by_author(const char* author, size_t* count, size_t maxCount)
{
	DocumentMeta meta = {0};
	meta.author = author;
	return sci_fill_meta(&meta, count, maxCount);
}

DocumentMeta* sci_find_by_title(const char* title)
{
	DocumentMeta meta = {0};
	meta.title = title;
	size_t count;
	DocumentMeta** documentMetas = sci_fill_meta(&meta, &count, 1);
	if(count > 0)
		return documentMetas[0]
	else
		return NULL;
}

DocumentMeta** sci_find_by_journal(const char* jornal, size_t* count, size_t maxCount)
{
	DocumentMeta meta = {0};
	meta.jornal = jornal;
	return sci_fill_meta(&jornal, count, maxCount);
}

bool sci_save_document_to_pdf(DocumentMeta* meta, const char* fileName)
{
	//TODO: implement
	assert(false);
}
