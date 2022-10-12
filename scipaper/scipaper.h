#pragma once

#include <stdbool.h>
#include "types.h"

bool sci_fill_meta(DocumentMeta* meta);
DocumentMeta* sci_find_by_doi(const char* doi);
DocumentMeta* sci_find_by_author(const char* author);
DocumentMeta* sci_find_by_title(const char* title);
DocumentMeta* sci_find_by_title(const char* jornal);

char* sci_get_document_text(DocumentMeta* meta);
unsigned char* sci_get_document_pdf_data(DocumentMeta* meta);
bool sci_save_document_to_pdf(DocumentMeta* meta, const char* fileName);

bool sci_paper_init();
void sci_paper_exit();
