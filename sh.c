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

char** getArgsFromInput(char *input)
{
  char** cmds = calloc(MAXARGS, sizeof(char*));
  char* token = strtok(input, " ");
  int i = 0;
  while(token != NULL){
    int len=strlen(token);
    cmds[i] = malloc(sizeof(char)*len+1);
    strcpy(cmds[i],token);
    token = strtok(NULL," ");
    i++;
  }
  return cmds;
}

void freeArgs(char** args){
   for(int i = 0; i < MAXARGS; i++){
     char* currentPtr = args[i];
     free(currentPtr);
   }
  free(args);
}

void fixNewLines(char** args){
  int i = 0;
  while(args[i]!=NULL){
    strtok(args[i],"\n");
    i++;
  }
}

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd, *cwd;
  char **args = calloc(MAXARGS, sizeof(char*));
  char **argsML = args;
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

  
	signal(SIGINT, handleSigInt);
	signal(SIGTSTP, handleSigStp);
	signal(SIGTERM, handleSigStp);
  
  while ( go )
  {
    freeArgs(args);
    /* print your prompt */
    printf("%s [%s]>", prompt, pwd);

    /* get command line and process */
    char input[BUFFERSIZE];
    if (fgets (input, BUFFERSIZE, stdin) == NULL)  
    {
      printf("\ncannot Ctrl-D\n");
    }
    else{
      args = getArgsFromInput(input);
      fixNewLines(args);
      command = args[0];
    }

    /* check for each built in command and implement */
    if (strcmp(command,"exit") == 0)  
    {
      printf("Executing built-in exit\n");
      free(prompt);
      free(commandline);
      free(owd);
      freePathElement(pathlist);
      free(pwd);
      freeArgs(args);
		  go = 0;
		}
		else if (strcmp(command,"which") == 0)   
    {
      printf("Executing built-in which\n");
			if (args[1] == NULL)
			{
				printf("which: too few arguments\n");
			}
			else
			{
        int argNumber = 1;
        while(args[argNumber] != NULL){
          char *path = which(args[argNumber], pathlist);
          if (path != NULL) 
          {
            printf("%s\n", path);
            free(path);
          } 
          else 
          {
            printf("%s %s: not found\n", args[0], args[argNumber]);
          }
          argNumber++;
        }
			}
		}
		else if (strcmp(command,"where") == 0)
    {
      printf("Executing built-in where\n");
      if (args[1] == NULL)
			{
				printf("where: too few arguments\n");
			}
			else
			{
        int argNumber = 1;
        while(args[argNumber] != NULL){
          char *path = where(args[argNumber], pathlist);
          if (path != NULL) 
          {
           printf("%s\n", path);
           free(path);
          } 
          else 
         {
           printf("%s %s: not found\n", args[0], args[argNumber]);
          }
          argNumber++;
        }
      } 
    }
    else if (strcmp(command,"cd") == 0)
    {
      printf("Executing built-in cd\n");
      if (args[2])
      {
				fprintf(stderr,"cd: too many arguments\n");
			}
			else if (args[1]) 
      {
				if (strcmp(args[1],"-") == 0)
        {
					strcpy(pwd,owd);
					free(owd);
					owd = getcwd(NULL,PATH_MAX+1);
					chdir(pwd);
				}
				else 
        {
					free(pwd);
					free(owd);
					owd = getcwd (NULL, PATH_MAX+1);
					chdir(args[1]);
					pwd = getcwd(NULL, PATH_MAX+1);
				}
			}
    }
    else if (strcmp(command,"pwd") == 0)
    {
      printf("Executing built-in pwd\n");
      printWorkingDirectory();
    }
    else if(strcmp(command,"list") == 0)
    {
      printf("Executing built-in list\n");
      if ((args[1] == NULL) && (args[2] == NULL))
			{
				list(owd);
			}
			else
			{
				for (int i = 1; i < MAXARGS; i++)
				{
					if (args[i] != NULL)
					{
						printf("[%s]:\n", args[i]);
						list(args[i]);
					}
				}
			}
    }
    else if(strcmp(command,"pid") == 0)
    {
      printf("Executing built-in pid\n");
      printf("shell PID: %d\n", getPID());
    }
    else if(strcmp(command,"kill") == 0)
    {
      printf("Executing built-in kill\n");
      if (args[1] != NULL && args[2] == NULL)
			{
				killProcess(atoi(args[1]), 0);
			}
			else if(args[1] != NULL && args[2] != NULL)
      {
				killProcess(atoi(args[2]), -1 * atoi(args[1]));
			}
    }
    else if(strcmp(command,"prompt") == 0)
    {
      printf("Executing built-in prompt\n");
      changePrompt(args,prompt);
    }
    else if(strcmp(command,"printenv") == 0)
    {
      printf("Executing built-in printenv\n");
      if (args[1] == NULL) 
      { 
        printenv(envp);
      }
      else if((args[1] != NULL) && (args[2] == NULL)) 
      { 
        printf("%s\n", getenv(args[1]));
      }
      else 
      {
        printf("printenv: too many arguments\n");
      }
    }
    else if(strcmp(command,"setenv") == 0)
    {
      printf("Executing built-in setenv\n");
      if(args[1] == NULL)
      {
        printenv(envp);
      }
      else if((args[1] != NULL) && (args[2] == NULL)) 
      { 
        setenv(args[1], "",1);
      }
      else if((args[1] != NULL) && (args[2] != NULL) && (args[3] == NULL)) 
      {
        setenv(args[1],args[2],1);

        if(strcmp(args[1], "HOME") == 0) 
        {
					homedir = getenv("HOME");
				}
				if(strcmp(args[1],"PATH") == 0) 
        {
					freePathElement(pathlist);
          free(pathlist);
					pathlist = get_path();
				}
			}
      else 
      { 
				perror("setenv");
				printf("setenv: too many arguments\n");
			}
    }
    /*  else  program to exec */
		else
    {
      /* find it */
			char* cmd=which(args[0],pathlist);
			int pid=fork();
			if (pid)
      {
				free(cmd);
				waitpid(pid,NULL,0);
			}
      /* else */
			else
      {
				if (execve(cmd, args, envp) < 0)
				{
          if(strcmp(strtok(args[0],"\n"),"")!=0){
					  fprintf(stderr, "%s: command not found.\n", args[0]);
          }
					exit(0);
				}
			}
		}
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
  char* cmd = (char *)malloc(BUFFERSIZE);
  while (pathlist) 
  { //WHERE
    sprintf(cmd, "%s/%s", pathlist->element, command);
    if (access(cmd, F_OK) == 0) 
    {
      return cmd;
    }
    pathlist = pathlist->next;
  }
  free(cmd);
  return NULL;
} /* where() */

void printWorkingDirectory()
{
  char currentWorkingDirectory[256];
	getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory));
  printf("%s\n", currentWorkingDirectory);
} /* printWorkingDirectory() */

void list (char *dir)
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
  DIR *directory;
  struct dirent *de;
  directory = opendir(dir);
  while ((de = readdir(directory)) != NULL) 
    {
      printf("%s\n", de->d_name);
    }
  closedir(directory);
} /* list() */

int getPID()
{
  return getpid();
} /* getPID() */

void killProcess(pid_t pid, int sig)
{
	if (sig == 0)
  {
		kill(pid,SIGTERM);
	}
	else 
  {
		kill(pid, sig);
	}
} /* killProcess() */

void changePrompt(char** args, char* promptAddress){
  if(args[1] == NULL)
  {
    printf("input new prompt prefix: ");
    fgets(promptAddress, BUFFERSIZE, stdin);
    promptAddress[(int)strlen(promptAddress) - 1] = '\0';
    
  }
  else
  {
    strcpy(promptAddress, args[1]);
  }
} /* changePrompt() */

void printenv(char **envp)
{
  int i = 0;
  while(envp[i] != NULL){
    printf("%s\n",envp[i]);
    i++;
  }
} /* printenv() */

void freePathElement(struct pathelement *pathElement)
{
  struct pathelement* currentPath = pathElement;
  while(currentPath!=NULL)
  {
    struct pathelement* tmp = currentPath->next;
    free(currentPath);
    currentPath = tmp;
  }
} /* freePathList() */

/* signal handler for Ctrl+C */
void handleSigInt(int sig)
{
	/* Reset handler to catch SIGINT next time.*/
	signal(SIGINT, handleSigInt);
	printf("\ncannot be terminated using Ctrl+C %d", waitpid(getPID(), NULL, 0));
	fflush(stdout);
	return;
}

/* signal handler for Ctrl+Z */
void handleSigStp(int sig)
{
	signal(SIGTSTP, handleSigStp);
	printf("\ncannot be terminated using Ctrl+Z");
	fflush(stdout);
}