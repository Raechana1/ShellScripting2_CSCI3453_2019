#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>

#define TOKEN_SIZE 64
#define DELIMITERS " \t\r\n\a"
#define BUFFERSIZE 1024

FILE * fd;
int cmd_cd(char **args);
int cmd_exit(char **args);
char *builtin_cmd[] = {"cd","exit"};
int (*functions[]) (char **) = {&cmd_cd, &cmd_exit};
int num_of_cmd() {return sizeof(builtin_cmd) / sizeof(char *);}



int cmd_cd(char **args){
	if (args[1] == NULL) {
		fprintf(stderr, "Expected argument to \"cd\"\n");
	} 
	else {
		if (chdir(args[1]) != 0) {
			perror("Error");
		}

	}
	return 1;
}

int cmd_exit(char **args){
	return 0;
}


int execute_cmd(char **args){
	int i;
	pid_t pid, wpid;
	int status;
	
	if (args[0] == NULL) {
		return 1;
	}
	
	for (i = 0; i < num_of_cmd(); i++) {
		//checks if there are commands that match the built in commands
		if (strcmp(args[0], builtin_cmd[i]) == 0) {
			return (*functions[i])(args);
		}
					
	}
	
	
	
	pid = fork();
	// Child process
	if (pid == 0) {
			//checks if > is used to redirect to file
		if (strcmp(args[1], ">") == 0){
			fd = fopen(args[2], "w+");
			args[2] = NULL;
			dup2(fileno(fd), fileno(stdout));
			fclose(fd);
			return 1;
		}

		//checks if < is used to redirect file to object
		if (strcmp(args[1], "<") == 0){
			fd = fopen(args[2], "r");
			args[2] = NULL;
			dup2(fileno(fd), fileno(stdin));
			fclose(fd);
			return 1;
		}
		if (execvp(args[0], args) == -1) 
			perror("Child Process cannot execute");
		exit(1);
	} 
	// Error forking
	else if (pid < 0){
		perror("Forking Error");
	} 
	
	// Parent process
	else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}
//reads the user input into a command in the buffer
char *read_cmd(void){
	int size = BUFFERSIZE;
	int index = 0;
	char *buffer = (char*)malloc(sizeof(char) * size);
	int character;
	if (!buffer) {
	fprintf(stderr, "lsh: allocation error\n");
	exit(1);
	}
	while (1) {
	// Read a character
	character = getchar();
	
	//checks if end of file or at new line
	if (character == EOF || character == '\n') {
	  buffer[index] = '\0';
	  return buffer;
	}
	else {
	  buffer[index] = character;
	}
	index++;
	// reallocate if over buffer size
	if (index >= size) {
	  size += BUFFERSIZE;
	  buffer = (char*)realloc(buffer, size);
	  if (!buffer) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(1);
	  }
	}
	}
}

//splits the command into tokens
char **parse_cmd(char *command){
	int size = TOKEN_SIZE, index = 0;
	
	//token pointer points to a pointer of allocated memory
	char **tokens = (char**)malloc(size * sizeof(char*));
	char *token;
	
	//Error if there are no tokens
	if (!tokens) {
		fprintf(stderr, "Allocatio error\n");
		exit(1);
	}
	token = strtok(command, DELIMITERS);
	
	//for when there is a token add to array
	while (token != NULL) {
		tokens[index] = token;
		index++;
		//reallocate if over buffer size
		if (index >= size) {
			size += TOKEN_SIZE;
			tokens = (char**)realloc(tokens, size * sizeof(char*));
			if (!tokens) {
			fprintf(stderr, "Allocation error\n");
			exit(1);
			}
		}
		//add delimiters
		token = strtok(NULL, DELIMITERS);
	}
	tokens[index] = NULL;
	return tokens;
}


void loop(void){
	char *command;
	char **args;
	int status;
	//prints out continuous prompt then 
	//reads user input, splits into parts
	//then execute
	do {
		printf("myshell> ");
		command = read_cmd();
		args = parse_cmd(command);
		status = execute_cmd(args);
		
		//clears up allocated memory used
		free(command);
		free(args);
	} while (status);
}
int main(int argc, char** argv){
	loop();
	return 0;
}