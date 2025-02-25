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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


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


	// Tokenize the input
	char *token = strtok(input, " \n");

	while(token){
		if(!strcmp(token,"#")){
			curr_command->argc = 0;
			break;
		} else if(!strcmp(token,"<")){
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

	// If line begins with blank line or comment, it will be ignored and the shell will reprompt
	// if(strcmp(curr_command->argv[0], "#")){
	// 	curr_command = NULL;
	// }

	// if(strcmp(token[0], "#")){
	// 	curr_command = NULL;
	// }

	// if(curr_command->argc == 0){
	// 	curr_command);
	// 	return NULL;
	// }

	//else{
		return curr_command;
	
}

// shell must kill any other processes or jobs that your shell 
// has started before it terminates itself.
void exit_command(struct command_line *curr_command)
{
	// printf("executing exit_command\n");
	exit(0);

}


void cd_command(struct command_line *curr_command)
{
	// printf("executing cd_command\n");
	
	// char * getcwd (char *cwdbuf, size_t size );
	// int chdir (const char *pathname);


	// printf("curr_command arg count: %d\n", curr_command->argc);

	// with no arguments - it changes to the directory specified in the HOME environment variable
	if(curr_command->argc == 1){
		
		// https://pubs.opengroup.org/onlinepubs/009696899/functions/getenv.html
		const char *home_ev = "HOME";
		char *home_directory;

		home_directory = getenv(home_ev);

		chdir(home_directory);
		 
	}

	// This command can also take one argument: the path of a directory to change to. Your cd command 
	// must support both absolute and relative paths.
	else if(curr_command->argc == 2){

		chdir(curr_command->argv[1]);
		
	}

}

void status_command(struct command_line *curr_command)
{
	// printf("executing status_command\n");
}


/*
 * 
 * Citation:
 * Adapted from Using exec() with fork() in Module 6
 * Source URL: https://canvas.oregonstate.edu/courses/1987883/pages/exploration-process-api-executing-a-new-program?module_item_id=24956220
*/
void other_commands(struct command_line *curr_command)
{
	// printf("executing other_commands\n");

	int childStatus;


	// Fork a new process
	pid_t spawnPid = fork();
  
	switch(spawnPid){
	case -1:
		perror("fork()\n");
		exit(1);
		break;
	case 0:
		// The child process executes this branch
		// printf("CHILD(%d) running command\n", getpid());


		// Source URL: Redirecting both Stdin and Stdout https://canvas.oregonstate.edu/courses/1987883/pages/exploration-processes-and-i-slash-o?module_item_id=24956228

		if (curr_command->input_file){

			// Open the source file
			int sourceFD = open(curr_command->input_file, O_RDONLY);
			if (sourceFD == -1) { 
			perror("source open()"); 
			exit(1); 
			}

			// Write the file descriptor to stdout
			// printf("File descriptor of input file = %d\n", sourceFD); 
		
			// Redirect stdin to source file
			int result = dup2(sourceFD, 0);
			if (result == -1) { 
			perror("source dup2()"); 
			exit(2); 
			}

		}

		if (curr_command->output_file){

			// Open target file
			int targetFD = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (targetFD == -1) { 
				perror("target open()"); 
				exit(1); 
			}
			
			// Write the file descriptor to stdout
			// printf("File descriptor of output file = %d\n", targetFD);

			// Redirect stdout to target file
			int result = dup2(targetFD, 1);
			if (result == -1) { 
				perror("target dup2()"); 
				exit(2); 
			}

		}

		execvp(curr_command->argv[0], curr_command->argv);
		
			// exec only returns if there is an error
			perror("execvp");
			exit(2);
			break;
		
		default:
		// The parent process executes this branch
		// Wait for child's termination
		spawnPid = waitpid(spawnPid, &childStatus, 0);
		// printf("PARENT(%d): child(%d) terminated\n", getpid(), spawnPid);
		// exit(0);
		break;
	} 
	
}



int main()
{
	struct command_line *curr_command;


	while(true)
	{
		curr_command = parse_input();

		if(curr_command->argc == 0){
			continue;
		}

		else if(strcmp(curr_command->argv[0], "exit") == 0){
			exit_command(curr_command);

		}

		else if(strcmp(curr_command->argv[0], "cd") == 0){

			cd_command(curr_command);

		}

		else if(strcmp(curr_command->argv[0], "status") == 0){
			status_command(curr_command);
		}
		
		else{
			other_commands(curr_command); 
		}

	}
	return EXIT_SUCCESS;
}