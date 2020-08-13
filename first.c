#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linenoise.h"

FILE* log_file;

void completion(const char *buf, linenoiseCompletions *lc) 
{
	fprintf (log_file, "buf: %s\n", buf);
	fflush  (log_file);

	if (!strncasecmp(buf,"settings w", 10)) 
	{
        linenoiseAddCompletion(lc,"settings write");
        linenoiseAddCompletion(lc,"settings wrong");
    }
	else
    if (!strncasecmp(buf,"settings r", 10)) 
	{
        linenoiseAddCompletion(lc,"settings read");
        linenoiseAddCompletion(lc,"settings reset");
    }
	else
    if (buf[0] == 's') 
	{
        linenoiseAddCompletion(lc,"settings");
    }
	else
    if (buf[0] == 'r') 
	{
        linenoiseAddCompletion(lc,"reset ");
    }
	else
    if (buf[0] == 'p') 
	{
        linenoiseAddCompletion(lc,"prbs");
        linenoiseAddCompletion(lc,"prbs 7");
        linenoiseAddCompletion(lc,"prbs 15");
        linenoiseAddCompletion(lc,"prbs 23");
        linenoiseAddCompletion(lc,"prbs 31");
        linenoiseAddCompletion(lc,"prbs read");
    }
}

char *hints(const char *buf, int *color, int *bold) {
    if (!strcasecmp(buf,"settings")) 
	{
        *color = 35;
        *bold = 0;
        return (char*) " read|write";
    }
    if (!strcasecmp(buf,"prbs")) 
	{
        *color = 35;
        *bold = 0;
        return (char*) " read|7|15|23|31";
    }
    return NULL;
}

int main(int argc, char **argv) {
    char *line;
    char *prgname = argv[0];

	log_file = fopen ("log.txt", "w");

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
