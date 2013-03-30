//commands
#define COMMANDPROMPT "rocketmon> "
#define CMDUNKNOWN "Command not recognized.\n"
#define SHOWTHREADID "show threadid"
#define SHOWSENSORS "show sensors"
#define SHOWMODULES "show modules"
#define TESTMOTORS "test motors"
#define HELP "help"
void *shellMain (void *arg);
int execCmd(char command[],char * cmdout);
