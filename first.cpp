#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linenoise.h"

//FILE* log_file;

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

// help strings
string top_help = 
"Available commands:\n instructions\n settings\n reset\n prbs\n exit\n"
"Type help [command] to see more infomation about a command\n"
"Type help instructions to learn how to use command line";
string no_help = "Help is not available for this item";
string settings_help = "Writes settings into specified address";
string reset_help = "Resets the device";
string prbs_help = "Sets all TX and RX units to transmit/receive specified PRBS pattern (7,15,23,31)";
string instructions_help = "Start typing a command.\n"
"A hint for the next parameter is shown as you type (if available).\n"
"Press TAB at any time to cycle though possible completions.\n"
"Press Enter to execute the command";
string missing_args = "Command is missing agruments. Syntax:\n settings write [addr] [data]";
string prbs_read_help = "Reads and displays PRBS counters from all RX units";

// menu structure record
// record structure:
// field 1: number of argument in command line that the user is typing
// field 2: string used for matching to user-typed input. Supports Regex
// field 3: hint for the NEXT argument (not the one currently being typed)
// field 4: pointer to callback function, or NULL if no callback
// field 5: pointer to string containing help message for this command. Last record contains pointer to top-level help
node_record nr[] = 
{
// lvl  command_text                hint for next cmd      callback	ptr to help string
	{0, "settings",                 "operation",           NULL,    &settings_help},
	{1, 	"write",                "address (dec or 0x)", NULL,    &missing_args},
	{2, 		"([0-9a-fx]+)",     "data (dec or 0x)",    NULL,    &missing_args},
	{3, 			"([0-9a-fx]+)", "<Enter>",             cb1,     &settings_help},
	{1, 	"wrong",                "stuff",               NULL,    &no_help},
	{2, 		"stuff",            "<Enter>",             NULL,    &no_help},
	{0, "reset",                    "<Enter>",             NULL,    &reset_help},
	{0, "prbs",                     "pattern|read",        NULL,    &prbs_help},
	{1, 	"(7|15|23|31)",         "<Enter>",             cb2,     NULL},
	{1, 	"read",                 "<Enter>",             cb2,     &prbs_read_help},
	{0,	"exit",                     "<Enter>",             cb3,     NULL},
	{0,	"instructions",             "<Enter>",             NULL,    &instructions_help},
	{-1,"help",                     "command",             NULL,    &top_help} // help command = end marker
};

int main(int argc, char **argv) 
{
   	char* buf;
	string line;

//	log_file = fopen ("linenoise_log.txt", "w");

	// construct linenoise
	linenoise ln(nr, "history.txt");

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to ln.prompt() will block as long as the user types something
     * and presses enter. */
    
    while((buf = ln.prompt("first> ")) != NULL) 
	{
		line = (string) buf;
		// index of the record when user pressed Enter
		int ei = ln.get_enter_index();

//		printf("cmd: '%s' enter index: %d\n", line.c_str(), ei);

		// call callback for this command if available
		if (ei >= 0 && nr[ei].cb != NULL) nr[ei].cb(line);

		// display help message if requested
		if (ei < 0)	printf("%s", ln.get_help_message().c_str());

		free (buf);
	}
    return 0;
}
