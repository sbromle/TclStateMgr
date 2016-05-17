#ifndef VARSTATE_H
#define VARSTATE_H

/*
 * This file is part of TclStateManager - a Tcl convenience wrapper
 * for managing binary blobs on C side from within Tcl.
 *
 * Maintain variable contexts on Tcl side.
 *
 * Copyright (C) 2003,2004,2011 Samuel P. Bromley <sam@sambromley.com>
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
 *
 */

#ifdef __cplusplus
extern "C" {
#endif 
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <tcl.h>

typedef int (*CreateObjFunc)
			(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST objv[], void *);
typedef int (*InstanceCommandFunc)
			(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST objv[]);
/* generic state management structure. Maps var names to blobs.
 * Created once per interpreter */
struct StateManager_s {
	Tcl_HashTable hash; /* list of variables by name */
	int uid;
	char *prefix;
	void (*deleteProc)(void *ptr);
	int (*unknownCmd)(ClientData, Tcl_Interp *,int,Tcl_Obj *CONST objv[]);
	int max_num_reg_types; /* 100 */
	int num_reg_types; /* 0 */
	char **reg_type_names; /* max+1 to end in NULL */
	CreateObjFunc *reg_types_create_procs;
	InstanceCommandFunc *reg_types_instance_commands;
};

/* generic state management structure. Maps var names to blobs.
 * Created once per interpreter */
typedef struct StateManager_s *StateManager_t;

extern int DLLEXPORT varExists0(StateManager_t statePtr, char *name);

extern int DLLEXPORT varExistsTcl(Tcl_Interp *interp,
		StateManager_t statePtr,
		Tcl_Obj *CONST name);

/** Return an array of all the elements in the hash, and the size of the array.
 * Caller is responsible for freeing the returned array. The elements should not
 * be freed, as they are still used by the hash table.
 */
extern int DLLEXPORT varElements(Tcl_Interp *interp, StateManager_t statePtr,
		ClientData **elements, int *len);
typedef int (*varSearchFunction)(ClientData element, ClientData clientData);
extern int DLLEXPORT varSearch(Tcl_Interp *interp, StateManager_t statePtr,
		varSearchFunction search_function,
		ClientData clientData, ClientData *result);
extern int DLLEXPORT varNames(Tcl_Interp *interp, StateManager_t statePtr);
extern int DLLEXPORT varNamesList(Tcl_Interp *interp, StateManager_t statePtr, Tcl_Obj **list);
/* generate a uniqe variable name */
extern int DLLEXPORT varUniqName(Tcl_Interp *interp, StateManager_t statePtr, char *name);

/* enumeration for registration modes */
typedef enum {REG_VAR_DELETE_OLD,
	REG_VAR_IGNORE_OLD} REG_VAR_MODE;

/* register data with state manager under name */
extern int DLLEXPORT
	registerVar(Tcl_Interp *interp, StateManager_t statePtr, ClientData data, char *name, REG_VAR_MODE mode);

/* de-register a variable and free its resources */
extern int DLLEXPORT varDelete0(Tcl_Interp *interp, StateManager_t statePtr,
		Tcl_Obj *objName);

extern int DLLEXPORT getVarFromObj(ClientData clientData, Tcl_Interp *interp, Tcl_Obj *CONST name,
		void **iPtrPtr);
extern int DLLEXPORT getVarFromObjKey(const char *state_key, Tcl_Interp *interp, Tcl_Obj *CONST name,
		void **iPtrPtr);

/* function to initialize state for a variable type */
extern int DLLEXPORT InitializeStateManager(Tcl_Interp *interp, const char *key,
		const char *cmd_name,
		int (*unknownCmd)(ClientData,Tcl_Interp*,int,Tcl_Obj *CONST objv[]),
		void (*deleteProc)(void *ptr));

#ifdef __cplusplus
}
#endif 
#endif
