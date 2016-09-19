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

typedef struct cObjStateContext *cObjStateContextPtr;

/** A data structure that hold cvobject (which can be a camera, kinect, img,
 * ptcloud ...) */
typedef struct cObj {
	char type_name[64]; /**< name of this object's type */
	uint64_t type_hash; /**< a hash of the name of the type */
	uint64_t refcount; /**< a reference count for optional resource management */
	void *object;
	void (*deleteFunc)(void *ptr);
	cObjStateContextPtr *context;
} cObj;

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

/* Function that must be called to initialize the state
 * manager.
 */
extern int  DLLEXPORT cObjState_Init(Tcl_Interp *interp);

// Means to retrieve objects in a friendly way;
extern int  DLLEXPORT getcObjFromObj(Tcl_Interp *interp, Tcl_Obj *CONST name,
		const char *type_name, cObj **iPtrPtr);

/* the command bound to each instance */
extern int  DLLEXPORT cObjInstanceCmd(ClientData data, Tcl_Interp *interp,
		    int objc, Tcl_Obj *CONST objv[]);

/* function to register new object types */
extern int  DLLEXPORT registerNewType(Tcl_Interp *interp, const char *type_name,
		int (*createObjFunc)
			(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST objv[], void *),
		int (*instanceCommand)
			(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST objv[])
		);


/* Hash a string to an integer using the FNV1a Hashing algorithm */
extern uint64_t  DLLEXPORT FNV1aHash(const char *str, int maxlen);
/* Convenience macro for type hashing */
#define TYPEHASH(a,b) FNV1aHash((a),(b))

#ifdef __cplusplus
}
#endif

#endif  //COBJ_STATE_H 
