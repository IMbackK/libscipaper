#pragma once

#include <stdbool.h>
#include "types.h"

DocumentMeta** sci_fill_meta(const DocumentMeta* meta, size_t* count, size_t maxCount);
DocumentMeta* sci_find_by_doi(const char* doi);
DocumentMeta* sci_find_by_title(const char* title);
DocumentMeta** sci_find_by_author(const char* author, size_t* count, size_t maxCount);
DocumentMeta** sci_find_by_journal(const char* jornal, size_t* count, size_t maxCount);

char* sci_get_document_text(const DocumentMeta* meta);
unsigned char* sci_get_document_pdf_data(const DocumentMeta* meta);
bool sci_save_document_to_pdf(const DocumentMeta* meta, const char* fileName);

bool sci_paper_init(const char* config_file);
void sci_paper_exit(void);
