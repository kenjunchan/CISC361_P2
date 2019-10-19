
#include "get_path.h"

int pid;
void inputToCharArray(char *input,char** cmds);
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list (char *dir);
void printWD();
void printPID();
void newPromptPrefix(char *cmd, char *prompt);
void printenv(char **envp);
void getUserStr(char* str);

#define PROMPTMAX 32
#define MAXARGS 10
