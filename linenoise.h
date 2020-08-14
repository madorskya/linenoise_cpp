/* linenoise.h -- VERSION 1.0
 *
 * Guerrilla line editing library against the idea that a line editing lib
 * needs to be 20,000 lines of C code.
 *
 * See linenoise.c for more information.
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2010-2014, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010-2013, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LINENOISE_H
#define __LINENOISE_H

#include <string>
#include <vector>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
#define MAX_CMD_LENGTH 10000
extern FILE* log_file;


// menu tree node class

typedef struct 
{
	int level; // node level. 0 = top node
	string data; // menu item name
	string hint; // hint for next field. Leave empty for automatic hint
} node_record;

// menu tree class
// constructs the tree from simple structure array
class menu_tree_t
{
	public:

		node_record* nr; // user's node record

		// menu tree constructor
		// uses node_record structure array as input
		menu_tree_t (node_record* inr)
		{
			nr = inr;
		};

		void print ()
		{
		};

		int exact_match_regex(string line, string ex)
		{
			boost::regex re (ex);
			boost::smatch m;
			int res = 0;
			if (boost::regex_search(line, m, re)) res = 1;
			if (line.compare(ex) == 0) res |= 2;
			return res;
		};

		int partial_match_regex(string line, string ex)
		{
			boost::regex re (ex);
			boost::smatch m;
			int res = 0;
			if (boost::regex_search(line, m, re)) res = 1;
			if (ex.find(line) != string::npos) res |= 2;
			return res;
		};

		vector <string> fld;
		vector <string> v;
		string matching_records; // list of matching fields
		string hints; // returned hints

		vector <string> find_matches (string pat)
		{
			boost::trim (pat);
			fld.clear();
			v.clear();
			matching_records.clear();
			int last_top_level = 0;
			int res;

			boost::split(fld, pat, boost::is_any_of(" ")); // split into fields
			size_t fld_size = fld.size();

			for (size_t i = 0; i < fld.size(); i++) // i = field in pat, aka menu level
			{
				// i is also tree branch level
				bool inside_branch = false;
				// scan all records at level i
				for (int j = last_top_level; ; j++) // j = nr record number
				{
					if (nr[j].level == -1) // last record, stop
						break;

					if (nr[j].level == (int)i) // correct level
					{
						inside_branch = true; // remember that we're inside correct branch
						// try matching user's input
						// if the number of fields is more than this level, require exact match
						if (fld_size-1 > i)
						{
							if ((res = exact_match_regex (fld[i], nr[j].data))) // pat field matches record
							{
								// found exact match in non-last field, more fields to process
								// if regex match, then add field itself as completion
								if (res & 1) matching_records += fld[i] + " ";
								else         matching_records += nr[j].data + " ";
								// remember number
								last_top_level = j;
								// can quit loop now
								break;
							}
						}
						else
						{
							// last level to analyze
							// fld[i] may contain partial match
							if ((res = partial_match_regex (fld[i], nr[j].data)))
							{
								// construct completion line
								string cl = matching_records;
								// if regex match, then add field itself as completion
								if (res & 1) cl += fld[i] + " ";
								else         cl += nr[j].data + " ";
								v.push_back (cl);
							}
						}
					}
					else
					{
						// stop if getting out of branch to lower level
						if ((nr[j].level < (int)i) && inside_branch) break;
					}
				}
			}
			return v;
		};

		string find_hints (string pat)
		{
			boost::trim (pat);
			fld.clear();
			hints.clear();
			int last_top_level = 0;
			int res;

			boost::split(fld, pat, boost::is_any_of(" ")); // split into fields
			size_t fld_size = fld.size();

			for (size_t i = 0; i < fld.size(); i++) // i = field in pat, aka menu level
			{
				// i is also tree branch level
				bool inside_branch = false;
				// scan all records at level i
				for (int j = last_top_level; ; j++) // j = nr record number
				{
					if (nr[j].level == -1) // last record, stop
						break;

					if (nr[j].level == (int)i) // correct level
					{
						inside_branch = true; // remember that we're inside correct branch
						// try matching user's input
						// if the number of fields is more than this level, require exact match
						if (fld_size-1 > i)
						{
							if ((res = exact_match_regex (fld[i], nr[j].data))) // pat field matches record
							{
								// found exact match in non-last field, more fields to process
								// remember number
								last_top_level = j;
								// can quit loop now
								break;
							}
						}
						else
						{
							// last level to analyze
							// fld[i] may contain partial match
							if ((res = exact_match_regex (fld[i], nr[j].data)))
							{
								// construct hint
								hints = nr[j].hint;
								return hints; // can get out immediately
							}
						}
					}
					else
					{
						// stop if getting out of branch to lower level
						if ((nr[j].level < (int)i) && inside_branch) break;
					}
				}
			}
			return hints;
		};
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct linenoiseCompletions {
  size_t len;
  char **cvec;
} linenoiseCompletions;

typedef void(linenoiseCompletionCallback)(const char *, linenoiseCompletions *);
typedef char*(linenoiseHintsCallback)(const char *, int *color, int *bold);
typedef void(linenoiseFreeHintsCallback)(void *);
void linenoiseSetCompletionCallback(linenoiseCompletionCallback *);
void linenoiseSetHintsCallback(linenoiseHintsCallback *);
void linenoiseSetFreeHintsCallback(linenoiseFreeHintsCallback *);
void linenoiseAddCompletion(linenoiseCompletions *, const char *);

char *linenoise(const char *prompt);
void linenoiseFree(void *ptr);
int linenoiseHistoryAdd(const char *line);
int linenoiseHistorySetMaxLen(int len);
int linenoiseHistorySave(const char *filename);
int linenoiseHistoryLoad(const char *filename);
void linenoiseClearScreen(void);
void linenoiseSetMultiLine(int ml);
void linenoisePrintKeyCodes(void);
void linenoiseMaskModeEnable(void);
void linenoiseMaskModeDisable(void);

#ifdef __cplusplus
}
#endif

#endif /* __LINENOISE_H */
