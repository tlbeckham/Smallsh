// Taylor Beckham
// CS374 
// Programming Assignment 4: SMALLSH

/*
* This program is a shell that provides a prompt for running commands, executes built in
* commands such as exit, cd, and status. It is also able to execute other commands by creating
* new processes using execvp(), supports input and output redirection, and supports running
* commands in the foreground and background processes.
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

/*
* Citation:
* Adapted from sample parser on assignment page
*/
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

	// record comment inputs as no args counted 
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


// shell must kill any other processes it has started then terminates itself
void exit_command(struct command_line *curr_command)
{
	exit(0);

}


void cd_command(struct command_line *curr_command)
{
	// command with no arguments changes to the directory specified in the HOME environment variable
	if(curr_command->argc == 1){
		
		/*
		* Citation:
		* Adapted from Getting the Value of an Environment Variable
		* Source URL: https://pubs.opengroup.org/onlinepubs/009696899/functions/getenv.html
		*/
		const char *home_ev = "HOME";
		char *home_directory;

		home_directory = getenv(home_ev);

		// change to home directory
		chdir(home_directory);
		 
	}

	// if command has an argument change to that directory
	else if(curr_command->argc == 2){

		chdir(curr_command->argv[1]);
		
	}

}

// track last exit code or signal status
int last_status = 0;

void status_command(struct command_line *curr_command)
{

	printf("exit value %d\n", last_status);

}


// struct for each background command 
struct bg_command {
	pid_t pid;
	char *command;
	int status;
	bool already_printed; // true or false if the background command has finished and been printed
	struct bg_command* next; // pointer to next element in the linked list
	};

// create a background command structure
struct bg_command* create_bg_command(pid_t pid, char *command, int status, bool already_printed) {

	struct bg_command* curr_bg_command = malloc(sizeof(struct bg_command));

	// Copy the value of pid into the pid in the structure
	curr_bg_command->pid = pid;

	// Allocate memory for command in the structure
	curr_bg_command->command = calloc(strlen(command) + 1, sizeof(char));
	// Copy the value of command into the command in the structure
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


void other_commands(struct command_line *curr_command)
{

	int childStatus;

	// Fork a new process
	pid_t childPid = fork();

	struct bg_command* new_bg_command;

	// both parent and child execute next instruction after fork
	switch(childPid){

	/*
	* Citation:
	* Adapted from Using exec() with fork() in Module 6
	* Source URL: https://canvas.oregonstate.edu/courses/1987883/pages/exploration-process-api-executing-a-new-program?module_item_id=24956220
	*/
	
	// fork failed
	case -1:
		last_status = 1;
		exit(1);
		break;

	// child process executes this branch
	case 0:
		// if theres an input file for redirection 
		if (curr_command->input_file){


			/*
			* Citation:
			* Adapted from Redirecting both Stdin and Stdout
			* Source URL: https://canvas.oregonstate.edu/courses/1987883/pages/exploration-processes-and-i-slash-o?module_item_id=24956228
			*/

			// Open the source file
			int sourceFD = open(curr_command->input_file, O_RDONLY);
			if (sourceFD == -1) { 
				printf("cannot open %s for input\n", curr_command->input_file);
				last_status = 1;
				exit(1); 
			}
		
			// Redirect stdin to source file
			int result = dup2(sourceFD, 0);
			if (result == -1) { 
				//perror("source dup2()"); 
				last_status = 2;
				exit(2); 
			}
		}

		// if theres an output file for redirection 
		if (curr_command->output_file){

			// Open target file
			int targetFD = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (targetFD == -1) { 
				//perror("target open()"); 
				last_status = 1;
				exit(1); 
			}

			// Redirect stdout to target file
			int result = dup2(targetFD, 1);
			if (result == -1) { 
				//perror("target dup2()"); 
				last_status = 2;
				exit(2); 
			}

		}

		// background commands are redirected to /dev/null if user doesn't redirect the standard input or output
		if (curr_command->is_bg){
			
			int sourceFD = open("/dev/null", O_RDONLY);
			if (sourceFD == -1) { 
				//perror("source open()"); 
				exit(1); 
			}

			int result = dup2(sourceFD, 0);
			if (result == -1) { 
				//perror("source dup2()"); 
				exit(2); 
			}

			int targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (targetFD == -1) { 
				//perror("target open()"); 
				exit(1); 
			}

			result = dup2(targetFD, 1);
			if (result == -1) { 
				//perror("target dup2()"); 
				exit(2); 
			}
		}

		execvp(curr_command->argv[0], curr_command->argv);
		
		// exec only returns if there is an error
		printf("%s: no such file or directory\n", curr_command->argv[0]);
		exit(2);
		break;

	// parent process executes this branch
	// Wait for child's termination
	default:
		if (!curr_command->is_bg){

			childPid = waitpid(childPid, &childStatus, 0);

			/*
			* Citation:
			* Adapted from Interpreting the Termination Status
			* Source URL: https://canvas.oregonstate.edu/courses/1987883/pages/exploration-process-api-monitoring-child-processes?module_item_id=24956219
			*/

			// determine how child exited
			if(WIFEXITED(childStatus)){
				last_status = WEXITSTATUS(childStatus);
			} 
			
			else{
				last_status = WTERMSIG(childStatus);
			}	

		}

		else if (curr_command->is_bg){

			printf("background pid is %d\n", childPid);

			// if background command then add to linked list to track
			new_bg_command = create_bg_command(childPid, curr_command->argv[0], childStatus, false);

			// Add current background command to the linked list
			if(head == NULL){
				// set the head and the tail to first element
				head = new_bg_command;
				tail = new_bg_command;
			} 

			else{
				// add node to the list and advance the tail
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

		// if bg command finsihed and status has not yet been printed to the terminal
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

		// check if any background processes have finished before returning command line
		bg_command_status(head);

		curr_command = parse_input();

		// re-prompt when blank line or comment is input to command line
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

