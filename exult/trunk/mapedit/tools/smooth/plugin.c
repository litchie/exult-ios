/*
 * author: Aurelien Marchand
 * licence: GPL
 * date: 20/06/03
 *
 */


#ifndef WIN32
#include <dlfcn.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "linked.h"

char *plug_error() {

	// Colourless: Unix specific part. Add your #def here
#ifdef WIN32
	HRESULT hRes;

	hRes = GetLastError();

	if (FAILED(hRes)) {
		static TCHAR lpMsgBuf[256];
		FormatMessage(
		    FORMAT_MESSAGE_FROM_SYSTEM |
		    FORMAT_MESSAGE_IGNORE_INSERTS,
		    NULL,
		    GetLastError(),
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    lpMsgBuf,
		    256,
		    NULL
		);
		return lpMsgBuf;
	} else
		return 0;
#else
	return(dlerror());
#endif
}

libhandle_t plug_load(char *plug_name) {
	libhandle_t a_hdl;

	// Colourless: Unix specific part. Add your #def here
#ifdef WIN32
	a_hdl = LoadLibrary(plug_name);
#else
	a_hdl = dlopen(plug_name, RTLD_LAZY);
#endif
	if (!a_hdl) {
		fprintf(stderr, "%s\n", plug_error());
		return(NULL);
	}

	return(a_hdl);

}

int plug_unload(libhandle_t a_hdl) {
	// in linux, dlclose return 0 when success and non-zero on error, so let's invert that to be consistant with the rest
	// in Windows, FreeLibrary returns 0 on failure, non zero on success
	// Colourless: Unix specific part. Add your #def here
#ifdef WIN32
	return FreeLibrary(a_hdl);
#else
	return(!dlclose(a_hdl));
#endif
}

void *plug_load_func(libhandle_t a_hdl, char *func_name) {
	// Colourless: Unix specific part. Add your #def here
#ifdef WIN32
	// Need to add _ to the start of the function names in windows
//	TCHAR strProcName[256] = "_";
//	strncat(strProcName, func_name, 256);
	return GetProcAddress(a_hdl, func_name);
#else
	return(dlsym(a_hdl, func_name));
#endif
};

int add_plugin_apply(int col_index, libhandle_t a_hdl) {
	// NOTE: It is necessary to add the handle AT THE END of the list for index col_index
	//       since the last transforming must have the priority (by design)

	// add a node to action_table[col_index] that points to handle's apply function

	// special case: action_table[col_index] == NULL

	// otherwise, cursor=action_table[col_index]
	// while (cursor->next) != NULL
	// cursor = cursor->next;
	// insert new node at cursor->next, with handle as the pointer to plugin_apply

	pfnPluginApply apply;
	//  char *(*apply)(colour_hex,glob_variables *);
	char *error;
	node *new_node;
	node *cursor; // used to navigate the list found at action_table[idx]

	// first of all, load a_hdl's apply function
	apply = plug_load_func(a_hdl, "plugin_apply");

	if ((error = plug_error()) != NULL)  {
		fprintf(stderr, "%s\n", error);
		return(-1);
	}
	if ((new_node = create_node()) == NULL) {
		// problem
		fprintf(stderr, "Couldn't create node\n");
		return(-1);
	}
	new_node->plugin_apply = (void *)apply;

	// add new_node at end of list found on action_table[idx]
	// dealing with the special case
	if (action_table[col_index] == NULL) {
		action_table[col_index] = new_node;
	} else {
		// go to the end and add it there
		cursor = action_table[col_index];
		while (cursor->next != NULL) {
			cursor = cursor->next;
		}
		// cursor->next == NULL
		cursor->next = new_node;
	}
	// all done
	return(0);
}


int add_plugin_parse(char *line, libhandle_t a_hdl) {
	int (*teach)(char *);
	char *error;

	teach = plug_load_func(a_hdl, "plugin_parse");
	if ((error = plug_error()) != NULL) {
		fprintf(stderr, "%s\n", error);
		return(-1);
	}
	return((*teach)(line));
}

node *add_handle(libhandle_t a_hdl, node *list) {
	// add a node to list that points to handle

	// create a node with a_hdl as node->a_hdl value and list as node->next value
	// insert node at head;

	node *new_head;
	if ((new_head = create_node()) == NULL) {
		// problem
		fprintf(stderr, "WARNING: couldn't create node\n");
		return(NULL);
	} else {
		new_head->next = list;
		new_head->handle = a_hdl;
	}

	return(new_head);
}

