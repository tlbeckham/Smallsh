// Taylor Beckham
// CS374 
// Programming Assignment 4: SMALLSH

/*
*
*
*
*/


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512


struct command_line
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};


struct command_line *parse_input()
{
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	// Get input
	printf(": ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

	// If line begins with blank line or comment, it will be ignored and the shell will reprompt
	// if (input[0] == '\n' || input[0] == '#') {
	// 	return NULL;
	// }

	// Tokenize the input
	char *token = strtok(input, " \n");
	while(token){
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	return curr_command;
}


void exit_command()
{
	printf("exit_command");
}


void cd_command()
{
	printf("cd_command");
}

void status_command()
{
	printf("status_command");
}



int main()
{
	struct command_line *curr_command;

	while(true)
	{
		curr_command = parse_input();

		if(strcmp(curr_command->argv[0], "exit") == 0){
			exit_command();

		}

		else if(strcmp(curr_command->argv[0], "cd") == 0){
			cd_command();

		}

		else if(strcmp(curr_command->argv[0], "status") == 0){
			status_command();
		}

	}
	return EXIT_SUCCESS;
}