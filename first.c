#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linenoise.h"

FILE* log_file;

// menu structure record
node_record nr[] = 
{
	{0, "settings", "operation"},
	{1, 	"write", "address"},
	{2, 		"([0-9a-fx]+)", "data"},
	{3, 			"([0-9a-fx]+)", "<Enter>"},
	{1, 	"wrong", "<Enter>"},
	{2, 		"stuff", "<Enter>"},
	{0, "reset", "<Enter>"},
	{0, "prbs", "pattern"},
	{1, 	"7", "<Enter>"},
	{1, 	"15", "<Enter>"},
	{1, 	"31", "<Enter>"},
	{-1, "", ""} // end marker
};

menu_tree_t * menu_tree; // make menu tree global for now
char res[MAX_CMD_LENGTH];

void completion(const char *buf, linenoiseCompletions *lc) 
{

	// scan the tree, find matches
	vector<string> matches = menu_tree->find_matches(string(buf));	

	// add all matches as possible completions
	for (std::vector<string>::iterator it = matches.begin() ; it != matches.end(); ++it)
        linenoiseAddCompletion(lc, it->c_str());
}

char *hints(const char *buf, int *color, int *bold) 
{
	fprintf (log_file, "****************buf: %s\n", buf);
	fflush  (log_file);

	string mhint = menu_tree->find_hints (string(buf));
    if (mhint.size() > 0) 
	{
        *color = 35;
        *bold = 1;
        strcpy(res, mhint.c_str());
		return res;
    }
    return NULL;
}

int main(int argc, char **argv) {
    char *line;
    char *prgname = argv[0];

	log_file = fopen ("log.txt", "w");

	// construct menu tree
	menu_tree = new menu_tree_t(nr);
	menu_tree->print();

    /* Parse options, with --multiline we enable multi line editing. */
    while(argc > 1) {
        argc--;
        argv++;
        if (!strcmp(*argv,"--multiline")) {
            linenoiseSetMultiLine(1);
            printf("Multi-line mode enabled.\n");
        } else if (!strcmp(*argv,"--keycodes")) {
            linenoisePrintKeyCodes();
            exit(0);
        } else {
            fprintf(stderr, "Usage: %s [--multiline] [--keycodes]\n", prgname);
            exit(1);
        }
    }

    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad("history.txt"); /* Load the history at startup */

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to linenoise() will block as long as the user types something
     * and presses enter.
     *
     * The typed string is returned as a malloc() allocated string by
     * linenoise, so the user needs to free() it. */
    
    while((line = linenoise("hello> ")) != NULL) {
        /* Do something with the string. */
        if (line[0] != '\0' && line[0] != '/') {
            printf("echo: '%s'\n", line);
            linenoiseHistoryAdd(line); /* Add to the history. */
            linenoiseHistorySave("history.txt"); /* Save the history on disk. */
        } else if (!strncmp(line,"/historylen",11)) {
            /* The "/historylen" command will change the history len. */
            int len = atoi(line+11);
            linenoiseHistorySetMaxLen(len);
        } else if (!strncmp(line, "/mask", 5)) {
            linenoiseMaskModeEnable();
        } else if (!strncmp(line, "/unmask", 7)) {
            linenoiseMaskModeDisable();
        } else if (line[0] == '/') {
            printf("Unreconized command: %s\n", line);
        }
        free(line);
    }
    return 0;
}
