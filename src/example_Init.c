/*
 * This file is part of statemgr - a library providing routines
 * to manage arbitrary binary structures within Tcl.
 *
 * Provide the Tcl package loading interface for a sample
 * extension.
 *
 * Copyright (C) 2011 Samuel P. Bromley <sam@sambromley.com>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License Version 3,
 * as published by the Free Software Foundation.
 *
 * It is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * (see the file named "COPYING"), and a copy of the GNU Lesser General
 * Public License (see the file named "COPYING.LESSER") along with this software.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>

#endif

/* string uniquely associating this state manager
 * to a Tcl interpreter */
#define SAMPLE_STATE_KEY "sample_state"

int Sample_Init(Tcl_Interp *interp) {
	/* initialize the stub table interface */
	if (Tcl_InitStubs(interp,"8.1",0)==NULL) {
		return TCL_ERROR;
	}
	
	/* string uniquely associating this state manager
 	* to a Tcl interpreter */
	char *key = SAMPLE_STATEKEY;
	/* name of the command user will use to interact with state */
	char *cmd_name="sample";
	/* pointer to command that will handle any unknown subcommand.
	 * This should at least support the "create" subcommand */
	int (*unknown_command_handler)(ClientData, Tcl_Interp,int objc, Tcl_Obj *CONST objv[])=
		my_object_cmd;
	/* pointer to function that can free resources of objects */
	void (*object_delete_func)(void *ptr)=my_object_delete_func;

	/* initialize the state manager. It will also create the base Tcl command
	 * for manipulating the state. */
	InitializeStateManager(interp,
			key,
			cmd_name,
			unknown_command_handler,
			object_delete_func);

	/* Declare that we provide the sample package */
	Tcl_PkgProvide(interp,"sample","1.0");
	return TCL_OK;
}
