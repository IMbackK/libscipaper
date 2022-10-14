#pragma once
#include "types.h"

int sci_plugin_register(const char* name, DocumentMeta** (*fill_meta_in)(const DocumentMeta* meta, size_t* count, size_t maxCount, void* user_data),
						char* (*get_document_text_in)(const DocumentMeta* meta, void* user_data),
						unsigned char* (*get_document_pdf_data_in)(const DocumentMeta* meta, void* user_data), void* user_data);

void sci_plugin_unregister(int id);

