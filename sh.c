#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"
#define BUFFERSIZE 128

void inputToCharArray(char *input,char** cmds)
{
  char* temp;
	temp=strtok(input," ");
	if (temp==NULL)
  {
    cmds[0]=malloc(sizeof(char));
		cmds[0][0]=0;
		return;
	}
  
  int len=strlen(temp);
	cmds[0]=malloc(sizeof(char)*len+1);
	strcpy(cmds[0],temp);
	int i=1;
	while ((temp=strtok(NULL," "))!=NULL)
  {
	  len=strlen(temp);
		cmds[i]=malloc(sizeof(char)*len+1);
		strcpy(cmds[i],temp);
		i++;
	}
		cmds[i]=NULL;
}

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		          /* Home directory to start out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();

  while ( go )
  {
    /* print your prompt */
    printf("%s%s$", prompt, pwd);

    /* get command line and process */
    char buffer[BUFFERSIZE];
    fgets (buffer, BUFFERSIZE, stdin);
		int len = strlen(buffer);
		buffer[len-1]=0;

    inputToCharArray(buffer, args);

    if (!strcmp(args[0],"exit"))
    {
      printf("EXITING\n");
		  go=0;
		}
		else if (!strcmp(args[0],"which"))
    {
			char *path = which(args[1], pathlist);
			if (args[1] == NULL)
			{
				printf("not enough arguments\n");
			}
			else
			{
				if (path)
				{
					printf("%s\n", path);
					free(path);
				}
				else
					printf("%s %s: not found\n", args[0], args[1]);
			}
		}
		else if (!strcmp(args[0],"where"))
    {
      char *path = which(args[1], pathlist);
			if (args[1] == NULL)
			{
				printf("not enough arguments\n");
			}
			else
			{
				if (path)
				{
					printf("%s\n", path);
					free(path);
				}
				else
					printf("%s %s: not found\n", args[0], args[1]);
			}   
    }
    else if (!strcmp(args[0],"pwd"))
    {
      printWD();
    }
    else if(!strcmp(args[0],"prompt"))
    {
      newPromptPrefix(args[1],prompt);
    }
		else{
			//call which to get the absolute path
			char* cmd=which(args[0],pathlist);
			int pid=fork();
			if (pid){
				free(cmd);
				waitpid(pid,NULL,0);
			}
			else{
				//try to exec the absolute path
				execve(cmd,args,envp);
				printf("exec %s\n",args[0]);
				exit(0);
			}
		}

    /* check for each built in command and implement */

    /*  else  program to exec */
    
    /* find it */
    
    /* do fork(), execve() and waitpid() */

    /* else */
    
    /* fprintf(stderr, "%s: Command not found.\n", args[0]); */
    
  }
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */
  char* cmd = (char *)malloc(BUFFERSIZE);
  while (pathlist) 
  { //WHICH
    sprintf(cmd, "%s/%s", pathlist->element, command);
    if (access(cmd, X_OK) == 0) 
    {
      return cmd;
    }
    pathlist = pathlist->next;
  }
  free(cmd);
  return NULL;
} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
  char *result = malloc(BUFFERSIZE);
	while (pathlist)
	{ // WHERE
		sprintf(result, "%s/%s", pathlist->element, command);
		if (access(command, F_OK) == 0)
			printf("[%s]\n", command);
		pathlist = pathlist->next;
	}
	printf(command, ": command not found");
	return NULL;
} /* where() */

void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
} /* list() */

void printWD()
{
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
  printf("%s\n", cwd);
}

void newPromptPrefix(char *command, char *p) 
{
  char buffer[BUFFERSIZE];
  int len;
  if (command == NULL) 
  {
    command = malloc(sizeof(char) * PROMPTMAX);
    printf("Input new prompt prefix: ");
    if (fgets(buffer, BUFFERSIZE, stdin) != NULL) {
    len = (int) strlen(buffer);
    buffer[len - 1] = '\0';
    strcpy(command, buffer);
    }
    strcpy(p, command);
    free(command);
  }
  else 
  {
    strcpy(p, command);
  }
}

/* void newPrompt(char *command, char *p)
{
  char buffer[BUFFERSIZE];
  int len;
  if(command==NULL)
  {
    command = malloc(sizeof(char)*PROMPTMAX);
    printf("Input new prompt prefix: ");
    if(fgets(buffer, BUFFERSIZE, stdin) != NULL)
    {
      len = (int) strlen(buffer);
      buffer[len-1] = '\0';
      strcpy(p, buffer);
      strcpy(p, command);
    }
    else
    {
      strcpy(p, command);
    }
    
  }
  free(command);
} */


