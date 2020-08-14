#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linenoise.h"

FILE* log_file;

void cb1 (string cmd)
{
	printf ("cb1 called: %s\n", cmd.c_str());
};

void cb2 (string cmd)
{
	printf ("cb2 called: %s\n", cmd.c_str());
};

void cb3 (string cmd)
{
	printf ("cb3 called: %s\n", cmd.c_str());
	exit (0);
};

// menu structure record
// record structure:
// field 1: number of argument in command line that the user is typing
// field 2: string used for matching to user-typed input. Supports Regex
// field 3: hint for the NEXT argument (not the one currently being typed)
// field 4: pointer to callback function, or NULL if no callback
node_record nr[] = 
{
	{0, "settings", "operation", NULL},
	{1, 	"write", "address", NULL},
	{2, 		"([0-9a-fx]+)", "data", NULL},
	{3, 			"([0-9a-fx]+)", "<Enter>", cb1},
	{1, 	"wrong", "<Enter>", NULL},
	{2, 		"stuff", "<Enter>", NULL},
	{0, "reset", "<Enter>", NULL},
	{0, "prbs", "pattern", NULL},
	{1, 	"7", "<Enter>", cb2},
	{1, 	"15", "<Enter>", cb2},
	{1, 	"31", "<Enter>", cb2},
	{0,	"exit", "<Enter>", cb3},
	{-1, "", "", NULL} // end marker
};

int main(int argc, char **argv) 
{
   	char* buf;
	string line;

	// construct linenoise
	linenoise ln(nr, "history.txt");

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to ln.prompt() will block as long as the user types something
     * and presses enter. */
    
    while((buf = ln.prompt("first> ")) != NULL) 
	{
		line = (string) buf;
		// record index when user pressed Enter
		int ei = ln.get_enter_index();

		printf("cmd: '%s' enter index: %d\n", line.c_str(), ei);

		// call callback for this command if available
		if (ei >= 0 && nr[ei].cb != NULL) nr[ei].cb(line);

		free (buf);
	}
    return 0;
}
