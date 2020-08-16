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
 * Reworked by Alex Madorsky
 *  - converted into C++ class
 *  - added menu structure decoder
 *  
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

#include <termios.h>
#include <string>
#include <vector>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
#define MAX_CMD_LENGTH 10000
//extern FILE* log_file;

// callback for node record
typedef void(linenoise_enter_callback)(string cmd_line);

// menu tree node structure

typedef struct 
{
	int level; // node level. 0 = top node
	string data; // menu item name
	string hint; // hint for next field. Leave empty for automatic hint
	linenoise_enter_callback *cb; // this gets called when user presses Enter
	string * help; // pointer to help string for this node
} node_record;

// menu tree class
// constructs the tree from simple structure array
class menu_tree_t
{
	public:

		node_record* nr; // user's node record
		vector <string> fld;
		vector <string> v;
		string matching_records; // list of matching fields
		string hints; // returned hints
		int enter_index;
		string help_message;
		string help_command;
		string default_help_message;
		string default_hint;

		menu_tree_t (){enter_index = -1;};

		void import_node_record (node_record* inr);
		int exact_match_regex(string line, string ex);
		int partial_match_regex(string line, string ex);
		vector <string> find_matches (string pat);
		string find_hints (string pat);
};


typedef struct linenoiseCompletions 
{
  size_t len;
  char **cvec;
} linenoiseCompletions;

#define LINENOISE_DEFAULT_HISTORY_MAX_LEN 100
#define LINENOISE_MAX_LINE 4096

void at_exit_wrapper    (void* ln);
void completion_wrapper (void* ln, const char* a, linenoiseCompletions* b);
char* hints_wrapper     (void* ln, const char* a, int* b, int* c);

class linenoise
{
public:
	typedef void(linenoiseCompletionCallback)(void*, const char *, linenoiseCompletions *);
	typedef char*(linenoiseHintsCallback)(void*, const char *, int *color, int *bold);
	typedef void(linenoiseFreeHintsCallback)(void *);

	char *unsupported_term[4];
	linenoiseCompletionCallback *completionCallback;
	linenoiseHintsCallback *hintsCallback;
	linenoiseFreeHintsCallback *freeHintsCallback;

	struct termios orig_termios; /* In order to restore at exit.*/
	int maskmode; /* Show "***" instead of input. For passwords. */
	int rawmode; /* For atexit() function to check if restore is needed*/
	int mlmode;  /* Multi line mode. Default is single line. */
	int atexit_registered; /* Register atexit just 1 time. */
	int history_max_len;
	int history_len;
	char **history;
	char res[MAX_CMD_LENGTH];
	string hist_filen;

	menu_tree_t menu_tree;

	linenoise (node_record* inr, string hist_fn);

	/* The linenoiseState structure represents the state during line editing.
	 *  * We pass this state to functions implementing specific editing
	 *   * functionalities. */
	struct linenoiseState {
		int ifd;            /* Terminal stdin file descriptor. */
		int ofd;            /* Terminal stdout file descriptor. */
		char *buf;          /* Edited line buffer. */
		size_t buflen;      /* Edited line buffer size. */
		const char *prompt; /* Prompt to display. */
		size_t plen;        /* Prompt length. */
		size_t pos;         /* Current cursor position. */
		size_t oldpos;      /* Previous refresh cursor position. */
		size_t len;         /* Current edited line length. */
		size_t cols;        /* Number of columns in terminal. */
		size_t maxrows;     /* Maximum num of rows used so far (multiline mode) */
		int history_index;  /* The history index we are currently editing. */
	};

	enum KEY_ACTION{
		KEY_NULL = 0,       /* NULL */
		CTRL_A = 1,         /* Ctrl+a */
		CTRL_B = 2,         /* Ctrl-b */
		CTRL_C = 3,         /* Ctrl-c */
		CTRL_D = 4,         /* Ctrl-d */
		CTRL_E = 5,         /* Ctrl-e */
		CTRL_F = 6,         /* Ctrl-f */
		CTRL_H = 8,         /* Ctrl-h */
		KEY_TAB = 9,        /* Tab - renamed due to name conflict */
		CTRL_K = 11,        /* Ctrl+k */
		CTRL_L = 12,        /* Ctrl+l */
		ENTER = 13,         /* Enter */
		CTRL_N = 14,        /* Ctrl-n */
		CTRL_P = 16,        /* Ctrl-p */
		CTRL_T = 20,        /* Ctrl-t */
		CTRL_U = 21,        /* Ctrl+u */
		CTRL_W = 23,        /* Ctrl+w */
		KEY_ESC = 27,       /* Escape - renamed due to name conflict*/
		BACKSPACE =  127    /* Backspace */
	};

	/* We define a very simple "append buffer" structure, that is an heap
	 * allocated string where we can append to. This is useful in order to
	 * write all the escape sequences in a buffer and flush them to the standard
	 * output in a single call, to avoid flickering effects. */
	struct abuf {
		char *b;
		int len;
	};

	void linenoiseAtExit(void);
	void refreshLine(struct linenoiseState *l);

	void linenoiseSetCompletionCallback(linenoiseCompletionCallback *);
	void linenoiseSetHintsCallback(linenoiseHintsCallback *);
	void linenoiseSetFreeHintsCallback(linenoiseFreeHintsCallback *);
	void linenoiseAddCompletion(linenoiseCompletions *, const char *);

	char *prompt(const char *prompt);
	void linenoiseFree(void *ptr);
	int  linenoiseHistoryAdd(const char *line);
	int  linenoiseHistorySetMaxLen(int len);
	int  linenoiseHistorySave(const char *filename);
	int  linenoiseHistoryLoad(const char *filename);
	void linenoiseClearScreen(void);
	void linenoiseSetMultiLine(int ml);
	void linenoisePrintKeyCodes(void);
	void linenoiseMaskModeEnable(void);
	void linenoiseMaskModeDisable(void);
	int  isUnsupportedTerm();
	int  enableRawMode(int);
	void disableRawMode(int);
	int  getCursorPosition(int, int);
	int  getColumns(int, int);
	void linenoiseBeep();
	void freeCompletions(linenoiseCompletions*);
	int  completeLine(linenoiseState*);
	void abInit(abuf*);
	void abAppend(abuf*, const char*, int);
	void abFree(abuf*);
	void refreshShowHints(abuf*, linenoiseState*, int);
	void refreshSingleLine(linenoiseState*);
	void refreshMultiLine(linenoiseState*);
	int  linenoiseEditInsert(linenoiseState*, char);
	void linenoiseEditMoveLeft(linenoiseState*);
	void linenoiseEditMoveRight(linenoiseState*);
	void linenoiseEditMoveHome(linenoiseState*);
	void linenoiseEditMoveEnd(linenoiseState*);
	void linenoiseEditHistoryNext(linenoiseState*, int);
	void linenoiseEditDelete(linenoiseState*);
	void linenoiseEditBackspace(linenoiseState*);
	void linenoiseEditDeletePrevWord(linenoiseState*);
	int  linenoiseEdit(int, int, char*, size_t, const char*);
	int  linenoiseRaw(char*, size_t, const char*);
	char *linenoiseNoTTY();
	void freeHistory();
	void completion(const char*, linenoiseCompletions*);
	char *hints(const char*, int*, int*);
	int  get_enter_index (){ int ind = menu_tree.enter_index; menu_tree.enter_index = -1; return ind;};
	string get_help_message ();
};


/* Debugging macro. */
#if 0
FILE *lndebug_fp = NULL;
#define lndebug(...) \
	do { \
		if (lndebug_fp == NULL) { \
			lndebug_fp = fopen("/tmp/lndebug.txt","a"); \
			fprintf(lndebug_fp, \
					"[%d %d %d] p: %d, rows: %d, rpos: %d, max: %d, oldmax: %d\n", \
					(int)l->len,(int)l->pos,(int)l->oldpos,plen,rows,rpos, \
					(int)l->maxrows,old_rows); \
		} \
		fprintf(lndebug_fp, ", " __VA_ARGS__); \
		fflush(lndebug_fp); \
	} while (0)
#else
#define lndebug(fmt, ...)
#endif



#endif /* __LINENOISE_H */
