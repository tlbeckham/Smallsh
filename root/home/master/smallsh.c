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

	if (input[0] == '#'){
		curr_command->argc = 0;
		return curr_command;
	}

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

int last_status = 0;

void status_command(struct command_line *curr_command)
{
	// printf("executing status_command\n");

	printf("exit value %d\n", last_status);

}


// struct for each background command 
struct bg_command {
	pid_t pid;
	char *command;
	int status;
	bool already_printed;
	struct bg_command* next; // pointer to next element in the linked list
	};


struct bg_command* create_bg_command(pid_t pid, char *command, int status, bool already_printed) {

	struct bg_command* curr_bg_command = malloc(sizeof(struct bg_command));

	// Copy the value of pid into the pid in the structure
	curr_bg_command->pid = pid;

	// Allocate memory for curr_command in the structure
	curr_bg_command->command = calloc(strlen(command) + 1, sizeof(char));
	// Copy the value of curr_command into the curr_command in the structure
	strcpy(curr_bg_command->command, command);

	// Copy the value of pid into the pid in the structure
	curr_bg_command->status = status;

	// Copy the value of already_printed into the already_printed in the structure
	curr_bg_command->already_printed = already_printed;

	// Set the next node to NULL
	curr_bg_command->next = NULL;
	return curr_bg_command;
}


// The head of the linked list
struct bg_command* head = NULL;
// The tail of the linked list
struct bg_command* tail = NULL;


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
	pid_t childPid = fork();

	struct bg_command* new_bg_command;
  
	// both parent and child execute next instruction after fork
	switch(childPid){
	
		case -1:
		perror("fork()\n");
		last_status = 1;
		exit(1);
		break;

	case 0:

		// Source URL: Redirecting both Stdin and Stdout https://canvas.oregonstate.edu/courses/1987883/pages/exploration-processes-and-i-slash-o?module_item_id=24956228

		if (curr_command->input_file){

			// Open the source file
			int sourceFD = open(curr_command->input_file, O_RDONLY);
			if (sourceFD == -1) { 
				// perror("source open()"); 

				printf("cannot open %s for input\n", curr_command->input_file);
				last_status = 1;
				exit(1); 
			}
		
			// Redirect stdin to source file
			int result = dup2(sourceFD, 0);
			if (result == -1) { 
				perror("source dup2()"); 
				last_status = 2;
				exit(2); 
			}
		}

		if (curr_command->output_file){

			// Open target file
			int targetFD = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (targetFD == -1) { 
				perror("target open()"); 
				last_status = 1;
				exit(1); 
			}

			// Redirect stdout to target file
			int result = dup2(targetFD, 1);
			if (result == -1) { 
				perror("target dup2()"); 
				last_status = 2;
				exit(2); 
			}

		}

		
		if (curr_command->is_bg){
			
			// If the user doesn't redirect the standard input for a background command, then standard input must be redirected to /dev/null.
			int sourceFD = open("/dev/null", O_RDONLY);
			if (sourceFD == -1) { 
				perror("source open()"); 
				exit(1); 
			}

			int result = dup2(sourceFD, 0);
			if (result == -1) { 
				perror("source dup2()"); 
				exit(2); 
			}

			// If the user doesn't redirect the standard output for a background command, then standard output must be redirected to /dev/null.
			int targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (targetFD == -1) { 
				perror("target open()"); 
				exit(1); 
			}

			result = dup2(targetFD, 1);
			if (result == -1) { 
				perror("target dup2()"); 
				exit(2); 
			}
		}

		execvp(curr_command->argv[0], curr_command->argv);
		
		// exec only returns if there is an error
		printf("%s: no such file or directory\n", curr_command->argv[0]);
		exit(2);
		break;
		
	default:
		// The parent process executes this branch
		
		// Wait for child's termination
		if (!curr_command->is_bg){

			childPid = waitpid(childPid, &childStatus, 0);


			// https://canvas.oregonstate.edu/courses/1987883/pages/exploration-process-api-monitoring-child-processes?module_item_id=24956219
			if(WIFEXITED(childStatus)){
				last_status = WEXITSTATUS(childStatus);
			} 
			
			else{
				last_status = WTERMSIG(childStatus);
			}	

		}

		else if (curr_command->is_bg){

			printf("background pid is %d\n", childPid);

			new_bg_command = create_bg_command(childPid, curr_command->argv[0], childStatus, false);

			// Add current background command to the linked list
			if(head == NULL){
				// This is the first element in the linked link
				// Set the head and the tail to this element
				head = new_bg_command;
				tail = new_bg_command;
			} 

			else{
				// This is not the first element. 
				// Add this element to the list and advance the tail
				tail->next = new_bg_command;
				tail = new_bg_command;
			}
		}
		
		break;
	} 
	
}

void bg_command_status(struct bg_command* list)
{

	struct bg_command* iList = list; // temp pointer to head of list

	 // loop through background commands struct linked list
	 while(iList != NULL){

		// check if bg command finished
		pid_t finished_bg_command = waitpid(iList->pid, &iList->status, WNOHANG);

		if(finished_bg_command > 0 && !iList->already_printed){

			if(WIFEXITED(iList->status)){
				last_status = WEXITSTATUS(iList->status);
				printf("background pid %d is done: exit value %d\n", iList->pid, last_status);
				iList->already_printed = true;
			} 
			
			else{
				last_status = WTERMSIG(iList->status);
				printf("background pid %d is done: terminated by signal %d\n", iList->pid, last_status);
				iList->already_printed = true;
			}	
			
		}

		iList = iList-> next;
    }

}


int main()
{
	struct command_line *curr_command;

	while(true)
	{

		bg_command_status(head);

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

