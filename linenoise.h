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
#include <regex>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
#define MAX_LEVEL 100 // max number of levels in menu
#define MAX_CMD_LENGTH 10000 // max length of one command

// menu tree node class
class node_t
{
	public:
		string data; // menu item name 
		string hint;
		// pointers to next-level menu items
		// if the vector is empty, this is a leaf
		vector <struct node_t *> items;

		//allocate new node 
		node_t (string idata, string ihint) 
		{ 
			data = idata;
			hint = ihint;
			items.clear();
		};

		// adding new item to list of items
		void add_item (node_t *nitem)
		{
			items.push_back (nitem);
		};

		void print(string indent)
		{
			printf ("%sname: %s nodes: %ld\n", indent.c_str(), data.c_str(), items.size());
			for (vector<node_t *>::iterator it = items.begin() ; it != items.end(); ++it)
				    (*it)->print(indent + " ");
		};

		vector<string> find_matches (string pat)
		{
			boost::trim (pat);
			vector<string> v;
			v.clear();
			for (vector<node_t *>::iterator it = items.begin() ; it != items.end(); ++it)
			{
				// check if each branch matches the pattern
				// if it does, ask it to scan itself
				string br_name = (*it)->data; // this is branch name

				// branch name is longer than pattern
				if (br_name.size() >= pat.size())
				{
//					if (br_name.find(pat) != string::npos)
					regex re (br_name);
					smatch m;
					if (regex_search(pat, m, re) || br_name.find(pat) != string::npos)
					{
						// this branch name is longer than pattern already, 
						// just return it as completion
						v.push_back(br_name);
					}
				}
				else // pattern is longer
				{
					if (pat.find(br_name) != string::npos)
					{
						// this branch matches, but pattern is longer
						// assume that it begins with complete branch name
						// and remove it from pattern
						string red_pat = pat.substr(br_name.size());
						vector<string> cv = (*it)->find_matches (red_pat); // scan the branch
						// add matches to return vector
						for (vector<string>::iterator cvit = cv.begin() ; cvit != cv.end(); ++cvit)
						{
							// attach branch name in front of each completion
							(*cvit) = br_name + " " + (*cvit); 
						}
						v.insert (v.end(), cv.begin(), cv.end());
					}
				}
			}
			return v;
		};

		string find_hints (string pat)
		{
			boost::trim (pat);
			string s = "";

			if (pat.compare(data) == 0) // pattern matches the name of this node
			{
				if (hint.size() > 0) // this node has its own hint
				{
					s = " " + hint;
				}
				else
				{
					// scan all children
					for (vector<node_t *>::iterator it = items.begin() ; it != items.end(); ++it)
					{
						string br_name = (*it)->data; // this is branch name
						s += br_name + "|"; // just add child's name to hint
					}
					s = " " + s; // need space in front
					s = s.substr(0, s.size() - 1); // remove last |
				}
			}
			else
			{
				if (pat.size() > data.size())
				{
					// pattern longer than name of this item
					// remove name of this item from pattern
					string red_pat = pat.substr(data.size());
					// scan children
					for (vector<node_t *>::iterator it = items.begin() ; it != items.end(); ++it)
					{
						s = (*it)->find_hints (red_pat); // scan the branch
						if (s.size() > 0) break;
					}
				}
			}
			return s;
		};
};

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
		// this array is used to remember current record # for each level 
		// during construction
		int      cur_rec[MAX_LEVEL];
		// this array remembers last node for each level
		node_t * cur_node[MAX_LEVEL];

		// menu tree constructor
		// uses node_record structure array as input
		menu_tree_t (node_record* nr)
		{
			// reset all current levels
			for (int i = 0; i < MAX_LEVEL; i++) cur_rec[i] = -1;

			// scan input structure
			for (int i = 0; ; i++)
			{
				int cur_level = nr[i].level;
				string cur_data = nr[i].data;
				string cur_hint = nr[i].hint;
				if (cur_level == -1) break; // end of record

				// create new node, store in cur_node
				cur_node[cur_level] = new node_t (cur_data, cur_hint);
				// also store number of record
				cur_rec[cur_level] = i; 
				// find node with higher level
				if (cur_level > 0)
				{
					cur_node[cur_level - 1]->add_item (cur_node[cur_level]);
				}
				// if cur_level == 0 that's top node, nothing to add it to 

				cur_rec[nr[i].level] = i; // remember last record number for each level
			}
		};

		void print ()
		{
			// just tell the top node to print itself
			cur_node[0]->print("");
		};

		vector <string> find_matches (string pat)
		{
			return cur_node[0]->find_matches (pat);
		};

		string find_hints (string pat)
		{
			return cur_node[0]->find_hints (pat);
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
