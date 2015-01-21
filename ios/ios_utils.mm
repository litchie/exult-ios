#include "ios_utils.h"
#import <Foundation/Foundation.h>
#include <cassert>

static char docs_dir[512];

const char* ios_get_documents_dir()
{
	if (docs_dir[0] == 0) {
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *docDirectory = [paths objectAtIndex:0];
		strcpy(docs_dir, docDirectory.UTF8String);
		printf("Documents: %s\n", docs_dir);
	//	chdir(docs_dir);
//		*strncpy(docs_dir, , sizeof(docs_dir)-1) = 0;
	}
	return docs_dir;
}
