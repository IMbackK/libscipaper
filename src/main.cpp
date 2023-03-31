/**
* libscipaper
* Copyright (C) 2023 Carl Klemm
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 3 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the
* Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
*/

#include <iostream>
#include <scipaper/scipaper.h>
#include <algorithm>
#include <cassert>

#include "log.h"
#include "options.h"

static constexpr size_t resultsPerPage = 200;

bool grabPapers(const DocumentMeta* meta, bool dryRun, bool savePdf, bool saveText, const std::filesystem::path& outDir, size_t maxCount)
{
	Log(Log::INFO)<<"Trying to download "<<maxCount<<" results";
	RequestReturn* req = sci_fill_meta(meta, nullptr, std::min(maxCount, resultsPerPage), 0);
	bool retried = false;
	if(req)
	{
		size_t pages = req->totalCount/resultsPerPage;
		size_t totalCount = req->totalCount;

		Log(Log::INFO)<<"Got "<<totalCount<<" results in "<<pages<<" pages";

		if(dryRun)
		{
			request_return_free(req);
			return true;
		}

		size_t processed = 0;
		for(size_t page = 0; page <= pages; ++page)
		{
			if(page != 0)
				req = sci_fill_meta(meta, nullptr, std::min(maxCount, resultsPerPage), page);
			if(!req)
			{
				if(!retried)
					--page;
				retried = true;
				continue;
			}
			else
			{
				retried = false;
			}

			Log(Log::INFO)<<"Processing page "<<page<<": "<<processed<<" of "<<req->totalCount<<
				", got "<<req->count<<" results this page";
			for(size_t i = 0; i < req->count; ++i)
			{
				if(req->documents[i])
				{
					std::filesystem::path jsonpath = outDir/(std::to_string(processed) + ".json");

					if(savePdf)
					{
						std::filesystem::path pdfpath = outDir/(std::to_string(processed) + ".pdf");
						bool ret = sci_save_document_to_file(req->documents[i], pdfpath.c_str());
						if(!ret)
							Log(Log::WARN)<<"Could not get pdf for document "<<jsonpath;
					}

					char* text = nullptr;
					if(saveText)
					{
						text = sci_get_document_text(req->documents[i]);
						if(!text)
							Log(Log::WARN)<<"Could not get text for document "<<jsonpath;
					}

					Log(Log::DEBUG)<<"saveing meta for "<<jsonpath.c_str();
					bool ret = document_meta_save(jsonpath.c_str(), req->documents[i], text);
					if(!ret)
						Log(Log::WARN)<<"Could not save document metadata"<<jsonpath;
				}
				else
				{
					Log(Log::WARN)<<"Document meta for result "<<i<<" of page "<<page<<" is empty";
				}
				++processed;
				if(maxCount > 0 && processed >= maxCount)
					break;
			}
			request_return_free(req);
			if(maxCount > 0 && processed >= maxCount)
				break;
		}
		return true;
	}

	Log(Log::WARN)<<"The backend found no results for your query";
	return false;
}

bool checkDir(const std::filesystem::path& outDir)
{
	if(!std::filesystem::is_directory(outDir))
	{
		if(!std::filesystem::create_directory(outDir))
		{
			std::cerr<<outDir<<" dose not exist and can not be created\n";
			return false;
		}
	}
	return true;
}

int main(int argc, char** argv)
{
	Log::level = Log::INFO;
	Config config;
	argp_parse(&argp, argc, argv, 0, 0, &config);

	if(Log::level == Log::DEBUG)
		sci_log_set_verbosity(LL_DEBUG);

	if(!sci_paper_init(nullptr, nullptr, 0))
	{
		Log(Log::ERROR)<<"could not init scipaper";
		return 1;
	}

	bool ret = checkDir(config.outDir);
	if(!ret)
		return 1;

	int backendId = 0;
	if(!config.backend.empty())
	{
		backendId = sci_backend_get_id_by_name(config.backend.c_str());
		if(backendId == 0)
		{
			Log(Log::ERROR)<<"libscipaper reports that the backend "<<config.backend<<" is not available";
			return 1;
		}
	}

	DocumentMeta queryMeta = {
		.doi = const_cast<char*>(config.doi.empty() ? nullptr : config.doi.c_str()),
		.title = const_cast<char*>(config.title.empty() ? nullptr : config.title.c_str()),
		.journal = const_cast<char*>(config.journal.empty() ? nullptr : config.journal.c_str()),
		.keywords = const_cast<char*>(config.keywords.empty() ? nullptr : config.keywords.c_str()),
		.abstract = const_cast<char*>(config.abstract.empty() ? nullptr : config.abstract.c_str()),
		.searchText = const_cast<char*>(config.text.empty() ? nullptr : config.text.c_str()),
		.hasFullText = config.fullText || config.savePdf,
		.backendId = backendId
	};

	size_t length;
	char* json = document_meta_get_json(&queryMeta, nullptr, &length);
	Log(Log::DEBUG)<<"Using document meta: "<<json;
	free(json);
	ret = grabPapers(&queryMeta, config.dryRun, config.savePdf, config.fullText, config.outDir, config.maxNumber);
	if(!ret)
		return 1;
	return 0;
}
