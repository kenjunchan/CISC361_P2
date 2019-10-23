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
  /*
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
  */
  
  char* temp;
  char** cmds = calloc(MAXARGS, sizeof(char*));
  /*
	temp=strtok(input," ");
	if (temp==NULL)
  {
    cmds[0]=malloc(sizeof(char));
		cmds[0][0]=0;
		return cmds;
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
  */
  
  char* token = strtok(input, " ");
  int i = 0;
  while(token != NULL){
    int len=strlen(token);
    cmds[i] = malloc(sizeof(char)*len+1);
    //cmds[i] = token;
    strcpy(cmds[i],token);
    token = strtok(NULL," ");
    i++;
  }
  return cmds;
}

void freeArgs(char** args){
  int i = 0;
  while(args[i]!=NULL){
    free(args[i]);
    i++;
  }
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
  /*
  cwd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(cwd, pwd, strlen(pwd));
  */
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();

	signal(SIGINT, handleSigInt);
	signal(SIGTSTP, handleSigStp);
	signal(SIGTERM, handleSigStp);

  while ( go )
  {
    freeArgs(args);
    //free(args);
    /* print your prompt */
    printf("%s [%s]>", prompt, pwd);

    /* get command line and process */
    //args = NULL;
    char input[BUFFERSIZE];
    fgets (input, BUFFERSIZE, stdin);
		//int len = strlen(input);
		//input[len-1]=0;
    args = getArgsFromInput(input);
    fixNewLines(args);
    command = args[0];
    /* check for each built in command and implement */
    if (strcmp(command,"exit") == 0)  
    {
      printf("Executing built-in exit\n");
      free(prompt);
      free(commandline);
      free(owd);
      freeElement(pathlist);
      free(argsML);
      free(pwd);
      free(cwd);
      free(args);
      freeArgs(args);
		  go=0;
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
        /*
        for (int i = 1; i < MAXARGS; i++) 
        {
          if (args[i] != NULL)
          {
            char *path = which(args[i], pathlist);
            if (path != NULL) 
            {
              printf("%s\n", path);
              free(path);
            } 
            else 
            {
              printf("%s %s: not found\n", args[0], args[i]);
            }
          }
          else
          {
            break;
          }
        }
        */
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
        for (int i = 1; i < MAXARGS; i++) 
        {
          if (args[i] != NULL)
          {
            char *path = where(args[i], pathlist);
            if (path != NULL) 
            {
              printf("%s\n", path);
              free(path);
            } 
            else 
            {
              printf("%s %s: not found\n", args[0], args[i]);
            }
          }
          else
          {
            break;
          }
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
      printWD();
    }
    else if(strcmp(command,"list") == 0)
    {
      printf("Executing built-in list\n");
      if ((args[1] == NULL) && (args[2] == NULL))
			{
        //list(cwd);
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
      printPID();
    }
    else if(strcmp(command,"kill") == 0)
    {
      printf("Executing built-in kill\n");
      //one argument
      if (args[1] != NULL && args[2] == NULL)
			{
				killPID(atoi(args[1]), 0);
			}
      //two arguments
			else if(args[1] != NULL && args[2] != NULL)
      {
				killPID(atoi(args[2]), -1*atoi(args[1]));
			}
    }
    else if(strcmp(command,"prompt") == 0)
    {
      printf("Executing built-in prompt\n");
      newPromptPrefix(args[1],prompt);
    }
    else if(strcmp(command,"printenv") == 0)
    {
      printf("Executing built-in printenv\n");
      //zero arguments
      if (args[1] == NULL) 
      { 
        printenv(envp);
      }
      //one argument
      else if((args[1] != NULL) && (args[2] == NULL)) 
      { 
        printf("%s\n", getenv(args[1]));
      }
      //more than one argument
      else 
      {
        perror("printenv");
        printf("printenv: too many arguments\n");
      }
    }
    else if(strcmp(command,"setenv") == 0)
    {
      printf("Executing built-in setenv\n");
      //zero arguments
      if(args[1] == NULL)
      {
        printenv(envp);
      }
      //one argument
      else if((args[1] != NULL) && (args[2] == NULL)) 
      { 
        setenv(args[1], "",1);
      }
      //two arguments
      else if((args[1] != NULL) && (args[2] != NULL) && (args[3] == NULL)) 
      {
        setenv(args[1],args[2],1);

        if(!strcmp(args[1], "HOME")) 
        {
					homedir = getenv("HOME");
				}
				if(!strcmp(args[1],"PATH")) 
        {
					free(pathlist);
					pathlist = get_path();
				}
			}
      //more than two arguments
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
			//get the absolute path from which
			char* cmd=which(args[0],pathlist);
			int pid=fork();
      /* do fork(), execve() and waitpid() */
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
    //free(args);
    //args = NULL;
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

void printWD()
{
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
  printf("%s\n", cwd);
} /* printWD() */

void list (char *dir)
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
  DIR *dr;
  struct dirent *de;
  dr = opendir(dir);
  if (dr == NULL) 
  {
    perror(dir);
  } 
  else 
  {
    while ((de = readdir(dr)) != NULL) 
    {
      printf("%s\n", de->d_name);
    }
  }
  closedir(dr);
} /* list() */

void printPID()
{
  int pid = getpid();
  printf("shell PID: %d\n", pid);
} /* printPID() */

void killPID(pid_t pid, int sig)
{
	if (sig == 0)
  {
		kill(pid,SIGTERM);
	}
	else 
  {
		kill(pid, sig);
	}
} /* kill() */

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
} /* newPromptPrefix() */

void printenv(char **envp)
{
  char **currEnv = envp;
  while (*currEnv)
  {
    printf("%s \n", *(currEnv++));
  }
} /* printenv() */

void freeElement(struct pathelement *pathElement)
{
  struct pathelement* currPath = pathElement;
  while(currPath!=NULL)
  {
    struct pathelement* tmp = currPath->next;
    free(currPath);
    currPath = tmp;
  }
} /* freePathList() */

/* signal handler for Ctrl+C */
void handleSigInt(int sig)
{
	/* Reset handler to catch SIGINT next time.*/
	signal(SIGINT, handleSigInt);
	printf("cannot be terminated using Ctrl+C %d \n", waitpid(getpid(), NULL, 0));
	fflush(stdout);
	return;
}

/* signal handler for Ctrl+Z */
void handleSigStp(int sig)
{
	signal(SIGTSTP, handleSigStp);
	printf("cannot be terminated using Ctrl+Z \n");
	fflush(stdout);
}