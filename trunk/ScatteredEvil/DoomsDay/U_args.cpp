#include <ctype.h>
#include <string.h>
#include "u_args.h"

#define SC_EMPTY_QUOTE		-1

void PrepareCmdArgs(cmdargs_t *cargs, char *lpCmdLine)
{
	int		i, len = strlen(lpCmdLine);

	// Prepare the data.
	memset(cargs, 0, sizeof(cmdargs_t));
	strcpy(cargs->cmdLine, lpCmdLine);
	// Prepare.
	for(i=0; i<len; i++)
	{
		// Whitespaces are separators.
		if(isspace(cargs->cmdLine[i])) cargs->cmdLine[i] = 0;
		if(cargs->cmdLine[i] == '\\' && (cargs->cmdLine[i+1] == '"' ||
			cargs->cmdLine[i+1] == '\\')) // Escape sequence?
		{
			memmove(cargs->cmdLine+i, cargs->cmdLine+i+1, sizeof(cargs->cmdLine)-i-1);
			len--;
			continue;
		}
		if(cargs->cmdLine[i] == '"') // Find the end.
		{
			int start = i;
			cargs->cmdLine[i] = 0;
			for(++i; i<len && cargs->cmdLine[i] != '"'; i++)
			{
				if(cargs->cmdLine[i] == '\\' && (cargs->cmdLine[i+1] == '"' ||
					cargs->cmdLine[i+1] == '\\')) // Escape sequence?
				{
					memmove(cargs->cmdLine+i, cargs->cmdLine+i+1, sizeof(cargs->cmdLine)-i-1);
					len--;
					continue;
				}
			}
			// Unterminated quotes?
			if(i == len) break;
			// An empty set of quotes?
			if(i == start+1) 
				cargs->cmdLine[i] = SC_EMPTY_QUOTE;
			else
				cargs->cmdLine[i] = 0;
		}
	}
	// Scan through the cmdLine and get the beginning of each token.
	cargs->argc = 0;
	for(i=0; i<len; i++)
	{
		if(!cargs->cmdLine[i]) continue;
		// Is this an empty quote?
		if(cargs->cmdLine[i] == SC_EMPTY_QUOTE)
			cargs->cmdLine[i] = 0;	// Just an empty string.
		cargs->argv[cargs->argc++] = cargs->cmdLine + i;
		i += strlen(cargs->cmdLine+i);
	}
}

