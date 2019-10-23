
#include "get_path.h"

int pid;
char** getArgsFromInput(char *input);
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list (char *dir);
void printWorkingDirectory();
int getPID();
void freeArgs(char** args);
void fixNewLines(char** args);
void killProcess(pid_t pid, int sig);
void changePrompt(char** args, char* promptAddress);
void printenv(char **envp);
void freePathElement(struct pathelement *pathElement);
void handleSigInt(int sig);
void handleSigStp(int sig);

#define PROMPTMAX 32
#define MAXARGS 10
