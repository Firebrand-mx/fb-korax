// Command line argument services.
#ifndef __COMMAND_LINE_ARGS_H__
#define __COMMAND_LINE_ARGS_H__

#define MAX_ARGS	128

typedef struct
{
	char cmdLine[512];
	int argc;
	char *argv[MAX_ARGS];
} cmdargs_t;

void PrepareCmdArgs(cmdargs_t *cargs, char *lpCmdLine);

#endif //__COMMAND_LINE_ARGS_H__