/*
 * main.c
 *
 * CSC360 Assignemnt 1
 * Runs a shell program
 * Author: Annie
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits.h>

struct Background{
	char cmd[100];
	char state;
	int pid;
};

struct Background bg[5] = { {.pid = -1, .state = 'R'}, {.pid = -1, .state = 'R'}, {.pid = -1, .state = 'R'}, {.pid = -1, .state = 'R'}, {.pid = -1, .state = 'R'}};


/*
 *
 * Function Declarations
 */
int execvp(const char * file, char * const argv[]);
char *strcpy(char *dest, const char *src);
char *strtok(char *str, const char *delim);
/*
 * parse
 * source: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html
 *
 * input:
 * string input of the command to be parsed
 * 
 * output:
 * zero terminated array of char pointers
 */
void parse(char *cmd, char **argv)
{
	while(*cmd != 0) {
		while(*cmd == ' ' || *cmd == '\t' || *cmd == '\n')
			*cmd++ = '\0'; //replace white space with 0
		*argv++ = cmd; //save the argument at a position in array
		while(*cmd != '\0' && *cmd != ' ' && *cmd != '\n' && *cmd != '\t')
			cmd++; //skip argument until sees non white space

	}
	*argv = '\0'; //mark end of argument list

}

void verifyParse(char **argv)
{
	int i = 0;
	for(i = 0; argv[i] != 0; i++)
		printf("%s ", *(argv + i));
	printf("\n");
}

void execute_external_command(char **argv)
{
	pid_t fpid;
	fpid = fork();

	if(fpid < 0){
		printf("ERROR: fork failed\n");
		exit(1);
	}

	if(fpid == 0){ //in child process
		if(execvp(argv[0], argv) < 0){
			printf("ERROR: Faild to execute command\n");
			exit(1);
		}
	}

	if(fpid > 0){//in parent process
		waitpid(fpid, NULL, 0);
		printf("\n");
	}
	
}

int execute_bg(char **argv)
{
	pid_t fpid;

	fpid = fork();

	if(fpid < 0){
		printf("ERROR: fork failed\n");
		exit(1);
	}

	if(fpid == 0){ //in child process
		if(execvp(argv[0], argv) < 0){
			printf("ERROR: Faild to execute command\n");
			exit(1);
		}
	}


	return fpid;
	
}

int add_bg(int child_pid){
	int i = 0;
	for(i = 0; i < 5; i++){
		if(bg[i].pid == -1){
			bg[i].pid = child_pid;
			break;
		}	
	}
	return i;
}

int num_bg_jobs(){
	int count = 0;
	for(int i = 0; i < 5; i++){
		if(bg[i].pid == -1)
			continue;
		count++;
	}
	return count;
}

void check_bg(){
	for(int i = 0 ; i < 5; i++){
		int pid2check = bg[i].pid;
		if(pid2check == -1)
			continue;
		int status = waitpid(pid2check, NULL, WNOHANG);
		if(status < 0){
			printf("ERROR: waitpid failed for child [%d]\n", pid2check);
			continue;
		}

		if(status == 0)
			continue;

		printf("%d[Terminated]: %s\n", i, bg[i].cmd);

		bg[i].pid = -1;
		bg[i].state = 'R';
	}

}

char *config_dir(){
	long size = pathconf(".", _PC_PATH_MAX);
	char *dir = getcwd(NULL, (size_t)size);
	char bracket = '>';
	strncat(dir, &bracket, 1);

	return dir;
}

int main ( void )
{
	for (;;)
	{
	

		char *dir = config_dir();
		char *cmd = readline (dir);
		char bg_cmd[100];
		strcpy(bg_cmd, cmd);

		char *argv[16]; //stores the parsed command line arguments
		parse(cmd, argv);


		check_bg();

		if(argv[0] == 0){
			free (cmd);
			free (dir);
			continue;
		}
		
		if (strcmp(argv[0], "exit") == 0){
			printf("take care\n");
			exit(0);

		}else if(strcmp(argv[0], "cd") == 0){
			if(chdir(argv[1]) < 0){
				printf("ERROR: can't find directory: %s\n", argv[1]);
				
				free(cmd);free(dir);
				continue;
			}

		}else if(strcmp(argv[0], "bg") == 0){
			if(num_bg_jobs() >= 5){
				printf("Max number of background jobs running\n");
				
				free(cmd);free(dir);
				continue;
			}
			int i;
			for(i = 0; argv[i+1] != 0; i++){
				argv[i] = argv[i+1];
			}
			argv[i] = 0;
			int child_pid = execute_bg(argv);
			int job_id = add_bg(child_pid);
			strcpy(bg[job_id].cmd, bg_cmd); 

		}else if(strcmp(argv[0], "bglist") == 0){
			
			for(int i = 0; i < 5; i++){
				if(bg[i].pid == -1){
					continue;
				}
				printf("%d[%c]: %s\n", i, bg[i].state, bg[i].cmd);
			}	
			printf("\nTotal Background jobs: %d\n", num_bg_jobs());

		}else if(strcmp(argv[0], "bgkill") == 0){
			char *pid2kill_str = argv[1];
			int pid2kill = atoi(pid2kill_str);
			if(pid2kill > 5 || bg[pid2kill].pid == -1){
				printf("ERROR: job %d doesn't exist\n", pid2kill);
				free(cmd);free(dir);continue;
			}

			if(bg[pid2kill].state == 'S')
				kill(bg[pid2kill].pid, SIGCONT);
			kill(bg[pid2kill].pid, SIGQUIT);
			bg[pid2kill].pid = -1;
			bg[pid2kill].state = 'R';
			printf("%d[killed]: %s\n", pid2kill, bg[pid2kill].cmd);

		}else if(strcmp(argv[0], "start") == 0){
			char *pid2kill_str = argv[1];
			int pid2kill = atoi(pid2kill_str);
			if(pid2kill > 5 || bg[pid2kill].pid == -1){
				printf("ERROR: job %d doesn't exist\n", pid2kill);
				free(cmd);free(dir);continue;
			}
			if(bg[pid2kill].state == 'R'){
				printf("ERROR: job %d already running\n", pid2kill);
				free(cmd);free(dir);continue;
			}

			kill(bg[pid2kill].pid, SIGCONT);
			bg[pid2kill].state = 'R';
			
		}else if(strcmp(argv[0], "stop") == 0){
			char *pid2kill_str = argv[1];
			int pid2kill = atoi(pid2kill_str);
			if(pid2kill > 5 || bg[pid2kill].pid == -1){
				printf("ERROR: job %d doesn't exist\n", pid2kill);
				free(cmd);free(dir);continue;
			}
			
			if(bg[pid2kill].state == 'S'){
				printf("ERROR: job %d already stopped\n", pid2kill);
				
				free(cmd);free(dir);
				continue;
			}


			kill(bg[pid2kill].pid, SIGSTOP);
			bg[pid2kill].state = 'S'; 
			
		}else{
			execute_external_command(argv);

		}
	
		free (cmd);
		free (dir);
	}	
}
