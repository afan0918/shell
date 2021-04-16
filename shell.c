#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * author Afan Chen. 
 */

static char *args[512];
pid_t pid;
int command_pipe[2];

#define READ 0
#define WRITE 1

int execute(char *cmd, int input, int first, int last);
void WaitChild(int n);
char line[500];
int n = 0;

void type_prompt();
int read_command(int input, int first, int last);

int main()
{
	printf("Hello!\nThis is Afan@Shell\nctrl-c or ctrl-\\ to quit\n");
	while (true)
	{
		type_prompt();
		
		if (!fgets(line, 1024, stdin))
			return 0;
   
		int input = 0;
		int first = 1;

		char *cmd = line;
		char *next = strchr(cmd, '|'); /* 找第一個 '|' */
		const char *file = "<>";

		if (strpbrk(cmd, file) != NULL)
		{
			system(cmd);
			continue;
		}

		while (next != NULL)
		{
			*next = '\0';
			input = execute(cmd, input, first, 0);

			cmd = next + 1;
			next = strchr(cmd, '|'); /* 找其他的 '|' */
			first = 0;
		}
		input = execute(cmd, input, first, 1);
		WaitChild(n);
		n = 0;
	}
	return 0;
}

void type_prompt()
{
  printf("Afan@Shell:$ ");
  fflush(NULL);//清空緩衝區，把字串print出來
}

int read_command(int input, int first, int last)
{
	int pipettes[2];

	pipe(pipettes);
	pid = fork();

	if (pid == 0)
	{
		if (first == 1 && last == 0 && input == 0)
		{
			dup2(pipettes[WRITE], STDOUT_FILENO);
		}
		else if (first == 0 && last == 0 && input != 0)
		{
			dup2(input, STDIN_FILENO);
			dup2(pipettes[WRITE], STDOUT_FILENO);
		}
		else
		{
			dup2(input, STDIN_FILENO);
		}

		if (execvp(args[0], args) == -1){
			printf("OOOOOOOh!It's wrong.\n");
			_exit(EXIT_FAILURE);
		}
	}

	if (input != 0)
		close(input);

	close(pipettes[WRITE]);

	if (last == 1)
		close(pipettes[READ]);

	return pipettes[READ];
}

void WaitChild(int n)
{
	int i;
	for (i = 0; i < n; i++)
		wait(NULL);
}

char* SkipWhite(char *s)
{
	while (isspace(*s))
		s++;
	return s;
}

void Split(char *cmd)
{
	cmd = SkipWhite(cmd);
	char *next = strchr(cmd, ' ');
	int i = 0;

	while (next != NULL)
	{
		next[0] = '\0';
		args[i] = cmd;
		i++;
		cmd = SkipWhite(next + 1);
		next = strchr(cmd, ' ');
	}

	if (cmd[0] != '\0')
	{
		args[i] = cmd;
		next = strchr(cmd, '\n');
		next[0] = '\0';
		i++;
	}

	args[i] = NULL;
}

int execute(char *cmd, int input, int first, int last)
{
	Split(cmd);
	if (args[0] != NULL)
	{
		n += 1;
		return read_command(input, first, last);
	}
	return 0;
}
