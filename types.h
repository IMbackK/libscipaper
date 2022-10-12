#pragma once
#include <time.h>
/*
@article{Duan_2021,
	doi = {10.1016/j.ensm.2021.05.047},
	url = {https://doi.org/10.1016%2Fj.ensm.2021.05.047},
	year = 2021,
	month = {oct},
	publisher = {Elsevier {BV}},
	volume = {41},
	pages = {24--31},
	author = {Yanzhou Duan and Jinpeng Tian and Jiahuan Lu and Chenxu Wang and Weixiang Shen and Rui Xiong},
	title = {Deep neural network battery impedance spectra prediction by only using constant-current curve},
	journal = {Energy Storage Materials}
}
*/

typedef struct _DocumentMeta {
	char* doi;
	char* url;
	struct timespec time;
	char* publisher;
	char* volume;
	char* pages;
	char* author;
	char* title;
	char* journal;
	char* keywords;
	char* pdfUrl;
	char* bibtex;

	int backendId;
	void* backendData;
	size_t backendDataLength;
} DocumentMeta;

DocumentMata* document_meta_copy(const DocumentMata* meta);
DocumentMata* document_meta_free(const DocumentMata* meta);
