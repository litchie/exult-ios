/**
 **	Test script parser.
 **/

#include <stdio.h>
#include "compile.h"

int main
	(
	int argc,
	char *argv[]
	)
	{
	if (argc < 2)
		{
		fprintf(stderr, "Usage:  test1 <filename>\n");
		return (-1);
		}
					// Compile/translate.
	Script_compiler script(argv[1]);
	return (script.get_error());
	}
