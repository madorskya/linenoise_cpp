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


int main(int argc, char **argv) 
{
    char *line;
    char *prgname = argv[0];

	// construct linenoise
	linenoise ln(nr);


    /* Parse options, with --multiline we enable multi line editing. */
    while(argc > 1) {
        argc--;
        argv++;
        if (!strcmp(*argv,"--multiline")) {
            ln.linenoiseSetMultiLine(1);
            printf("Multi-line mode enabled.\n");
        } else if (!strcmp(*argv,"--keycodes")) {
            ln.linenoisePrintKeyCodes();
            exit(0);
        } else {
            fprintf(stderr, "Usage: %s [--multiline] [--keycodes]\n", prgname);
            exit(1);
        }
    }

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    ln.linenoiseHistoryLoad("history.txt"); /* Load the history at startup */

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to linenoise() will block as long as the user types something
     * and presses enter.
     *
     * The typed string is returned as a malloc() allocated string by
     * linenoise, so the user needs to free() it. */
    
    while((line = ln.prompt("first> ")) != NULL) 
	{
        /* Do something with the string. */
        if (line[0] != '\0' && line[0] != '/') {
            printf("echo: '%s'\n", line);
            ln.linenoiseHistoryAdd(line); /* Add to the history. */
            ln.linenoiseHistorySave("history.txt"); /* Save the history on disk. */
        } else if (!strncmp(line,"/historylen",11)) {
            /* The "/historylen" command will change the history len. */
            int len = atoi(line+11);
            ln.linenoiseHistorySetMaxLen(len);
        } else if (!strncmp(line, "/mask", 5)) {
            ln.linenoiseMaskModeEnable();
        } else if (!strncmp(line, "/unmask", 7)) {
            ln.linenoiseMaskModeDisable();
        } else if (line[0] == '/') {
            printf("Unreconized command: %s\n", line);
        }
        free(line);
    }
    return 0;
}
