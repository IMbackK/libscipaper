#pragma once

int sci_plugin_register(const char* name, DocumentMeta* (*fill_meta_in)(DocumentMeta* meta),
						char* (*get_document_text_in)(DocumentMeta* meta),
						unsigned char* (*sci_get_document_pdf_data_in)(DocumentMeta* meta));

void sci_plugin_unregister(int id);

