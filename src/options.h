/**
* papergrabber
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

#pragma once
#include <string>
#include <vector>
#include <argp.h>
#include <iostream>
#include <filesystem>
#include "log.h"
#include "types.h"

const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "<carl@uvos.xyz>";
static char doc[] = "Application that grabs text or pdf files for documents from online resources using libscipaper";
static char args_doc[] = "";

static struct argp_option options[] =
{
  {"verbose",			'v', 0,			0,	"Show debug messages" },
  {"quiet",				'q', 0,			0,	"Show only errors"},
  {"key-words",			'k', "[FILE]",	0,	"Search in key words" },
  {"title",				't', "[STRING]",0,	"Search in title"},
  {"jornal",			'j', "[STRING]",0,	"Search in journal"},
  {"abstract",			'a', "[STRING]",0,	"Search in abstract"},
  {"author",			'u', "[STRING]",0,	"Search for an author"},
  {"text",				'e', "[STRING]",0,	"Freeform text search"},
  {"doi",				'i', "[STRING]",0,	"Search for a specific doi" },
  {"dry-run",			'd', 0,			0,	"Just show how many results there are"},
  {"out-dir",			'o', "[DIRECTORY]",	0,	"Place to save output" },
  {"limit",				'l', "[NUMBER]",	0,	"Maximum number of results to process" },
  {"pdf",				'p', 0,				0,		"Save pdf"},
  {"full-text",			'f', 0,				0,		"Save full text"},
  {"backend",			'b', "[STRING]",	0,		"Ask scipaper to use a specific backend"},
  {"biblatex",			'x', 0,	0,		"metadata in biblatex format"},
  {"print",				'r', 0,	0,		"print metadata only, dont save anything"},
  {"short",				's', 0,	0,		"only print or save the title and the doi of eatch result"},
  {"sort",				's', 0,	0,		"only print or save the title and the doi of eatch result"},
  { 0 }
};

struct Config
{
	std::string keywords;
	std::string title;
	std::string journal;
	std::string abstract;
	std::string text;
	std::string doi;
	std::string backend;
	std::string author;
	std::filesystem::path outDir = "./out";
	size_t maxNumber = 10;
	bool dryRun = false;
	bool fullText = false;
	bool savePdf = false;
	bool biblatex = false;
	bool print = false;
	bool titleDoi = false;
	sorting_mode_t sortMode = SCI_SORT_RELEVANCE;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	Config *config = reinterpret_cast<Config*>(state->input);

	switch (key)
	{
	case 'v':
		Log::level = Log::DEBUG;
		break;
	case 'q':
		Log::level = Log::ERROR;
		break;
	case 'k':
		config->keywords.assign(arg);
		break;
	case 't':
		config->title.assign(arg);
		break;
	case 'o':
		config->outDir.assign(arg);
		break;
	case 'j':
		config->journal.assign(arg);
		break;
	case 'a':
		config->abstract.assign(arg);
		break;
	case 'u':
		config->author.assign(arg);
		break;
	case 'e':
		config->text.assign(arg);
		break;
	case 'd':
		config->dryRun = true;
		break;
	case 'l':
		config->maxNumber = stoll(std::string(arg));
		break;
	case 'b':
		config->backend.assign(arg);
		break;
	case 'i':
		config->doi.assign(arg);
		break;
	case 'f':
		config->fullText = true;
		break;
	case 'p':
		config->savePdf = true;
		break;
	case 'x':
		config->biblatex = true;
		break;
	case 'r':
		config->print = true;
		break;
	case 's':
		config->titleDoi = true;
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};
