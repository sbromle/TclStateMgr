/*
 * This file is part of TclStateManager module.
 *
 * Maintain contexts/handles to Objects at C-pointers.
 *
 * Copyright (C) 2011 Whitecap Scientific Corporation
 * Based heavily on the object handler from Sam Bromley's MVTH.
 * Original author: Richard Charron <rcharron@whitecapscientific.com>
 * Contributors: Sam Bromley <sbromley@whitecapscientific.com>
 *
 * TclStateManager is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License Version 3,
 * as published by the Free Software Foundation.
 *
 * TclStateManager is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * (see the file named "COPYING"), and a copy of the GNU Lesser General
 * Public License (see the file named "COPYING.LESSER") along with
 * TclStateManager. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef OPENCV_OBJ_STATE_H
#define OPENCV_OBJ_STATE_H

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdint.h>
#include <tcl.h>
#include "variable_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** A data structure that hold cvobject (which can be a camera, kinect, img,
 * ptcloud ...) */
typedef struct cObj {
	char type_name[64]; /**< name of this object's type */
	uint64_t type_hash; /**< a hash of the name of the type */
	void *object;
	void (*deleteFunc)(void *ptr);
} cObj;

typedef int (*CreateObjFunc)
			(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST objv[], cObj **);
typedef int (*InstanceCommandFunc)
			(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST objv[]);
/* a structure passed to clientData of Object commands,
 * which holds the overall Object states (to provide
 * access to other Objectvariables) as well
 * as a pointer to the particular Object bound to
 * the command being executed.
 */
typedef struct ObjCmdClientData {
	StateManager_t state;
	cObj *mSelf;
	InstanceCommandFunc instanceCommand;
} ObjCmdClientData;

// Means to retrieve objects in a friendly way;
extern int getcObjFromObj(Tcl_Interp *interp, Tcl_Obj *CONST name,
		const char *type_name, cObj **iPtrPtr);

/* the command bound to each instance */
extern int cObjInstanceCmd(ClientData data, Tcl_Interp *interp,
		    int objc, Tcl_Obj *CONST objv[]);

/* function to register new object types */
extern int registerNewType(Tcl_Interp *interp, const char *type_name,
		int (*createObjFunc)
			(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST objv[], cObj **),
		int (*instanceCommand)
			(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST objv[])
		);
#ifdef __cplusplus
}
#endif

#endif  //COBJ_STATE_H 
