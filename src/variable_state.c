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
 * TclStateManager.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <tcl.h>
#include "variable_state.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

/* this is called when the command associated with a state is destroyed.
 * The hash table is walked, destroying all variables as
 * you go, and then the HashTable itself is freed */
void StateManagerDeleteProc(ClientData clientData) {
	Tcl_HashEntry *entryPtr;
	Tcl_HashSearch search;
	void *iPtr=NULL;
	StateManager_t state=(StateManager_t)clientData;
	if (state==NULL) return;
	if (state->deleteProc==NULL) return;

	entryPtr=Tcl_FirstHashEntry(&state->hash,&search);
	while (entryPtr!=NULL) {
		iPtr=Tcl_GetHashValue(entryPtr);
		state->deleteProc(iPtr);
		Tcl_DeleteHashEntry(entryPtr);
		/* get the first entry again, not the next one, since
		 * the previous first one is now deleted. */
		entryPtr=Tcl_FirstHashEntry(&state->hash,&search);
	}
	ckfree((char*)state);
	return;
}

int varExists0(StateManager_t statePtr,
		char *name)
{
	Tcl_HashEntry *entryPtr=NULL;
	if (name==NULL) return 0;
	entryPtr=Tcl_FindHashEntry(&statePtr->hash,name);
	if (entryPtr==NULL) return 0;
	return 1;
}

int varExistsTcl(Tcl_Interp *interp,
		StateManager_t statePtr,
		Tcl_Obj *CONST name)
{
	if (varExists0(statePtr,Tcl_GetString(name)))
	{
		Tcl_SetObjResult(interp,Tcl_NewIntObj(1));
		return TCL_OK;
	}
	Tcl_SetObjResult(interp,Tcl_NewIntObj(0));
	return TCL_OK;
}

int varNamesList(Tcl_Interp *interp, StateManager_t statePtr, Tcl_Obj **list)
{   
	Tcl_HashEntry *entryPtr;
	Tcl_HashSearch search;
	Tcl_Obj *listPtr;
	char *name;

	/* Walk the hash table and build a list of names */
	listPtr=Tcl_NewListObj(0,NULL);
	entryPtr=Tcl_FirstHashEntry(&statePtr->hash,&search);
	while (entryPtr!=NULL) {
		name=Tcl_GetHashKey(&statePtr->hash,entryPtr);
		if (Tcl_ListObjAppendElement(interp,listPtr,
					Tcl_NewStringObj(name,-1))!=TCL_OK) return TCL_ERROR;
		entryPtr=Tcl_NextHashEntry(&search);
	}
	*list=listPtr;
	return TCL_OK;
}

#define Tcl_HashSize(tablePtr) ((tablePtr)->numEntries)

int varElements(Tcl_Interp *interp, StateManager_t statePtr, ClientData **elements, int *len)
{
	Tcl_HashEntry *entryPtr;
	Tcl_HashSearch search;
	int i;
	int nelements;
	ClientData *es=NULL;
	ClientData val=NULL;


	nelements=Tcl_HashSize(&statePtr->hash);
	if (nelements==0) {
		*elements=NULL;
		*len=0;
		return TCL_OK;
	}
	*len=nelements;
	es=(ClientData*)calloc(nelements,sizeof(ClientData));
	*elements=es;

	/* Walk the hash table and store the data */
	entryPtr=Tcl_FirstHashEntry(&statePtr->hash,&search);
	i=0;
	while (entryPtr!=NULL) {
		val=Tcl_GetHashValue(entryPtr);
		es[i]=val;
		entryPtr=Tcl_NextHashEntry(&search);
		i++;
	}
	return TCL_OK;
}

int varNames(Tcl_Interp *interp, StateManager_t statePtr)
{
	Tcl_Obj *list=NULL;
	if (varNamesList(interp, statePtr,&list)!=TCL_OK) return TCL_ERROR;
	Tcl_SetObjResult(interp,list);
	return TCL_OK;
}

int varUniqName(Tcl_Interp *interp, StateManager_t statePtr, char *name)
{
	char tmp[1024]="";
	if (statePtr==NULL) return TCL_ERROR;
	while (1) {
		snprintf(tmp,sizeof(tmp),statePtr->prefix,statePtr->uid);
		if (!varExists0(statePtr,tmp)) {
			strcpy(name,tmp);
			return TCL_OK;
		}
		/* otherwise, increment the uid and try again */
		statePtr->uid++;
	};
	return TCL_ERROR;
}

/* Search the elements in a hash, using a search function. The search function
 * returns true when a match is found (or when progress should stop).
 * Search function must have the prototype:
 * int (*searchFunc)(ClientData element, ClientData clientData)
 * where "element" will be the element passed from the Hash table, and "clientData"
 * is any other data to be passed to the search function during the search.
 *
 * For example, let's assume one is storing pointers to integers in the hash,
 * and one wishes to look for the first integer greater than some value,
 * we could define a search function as:
 * int int_thresh(ClientData element, ClientData threshPtr) {
 *   int thresh=*((int*)threshPtr);
 *   int val=*((int*)element);
 *   if (val>thresh) return 1;
 *   return 0;
 * }
 * and then call varSearch as:
 * int thresh=5;
 * varSearch(interp,statePtr,int_thresh,&thresh,&found);
 * if (found!=NULL) {
 *   printf("we found it!\n");
 * }
 */
int varSearch(Tcl_Interp *interp, StateManager_t statePtr,
		varSearchFunction searchFunc,
		ClientData clientData,
		ClientData *result)
{
	Tcl_HashEntry *entryPtr;
	Tcl_HashSearch search;
	ClientData val=NULL;

	/* Walk the hash table and perform the test */
	entryPtr=Tcl_FirstHashEntry(&statePtr->hash,&search);
	while (entryPtr!=NULL) {
		val=Tcl_GetHashValue(entryPtr);
		if (searchFunc(val,clientData)==1) {
			*result=val;
			return TCL_OK;
		}
		entryPtr=Tcl_NextHashEntry(&search);
	}
	return TCL_OK;
}


int registerVar(Tcl_Interp *interp, StateManager_t statePtr,
		ClientData data, char *name,
		REG_VAR_MODE mode)
{
	int new;
	Tcl_HashEntry *entryPtr;
	entryPtr=Tcl_CreateHashEntry(&statePtr->hash,name,&new);
	if (new!=1) {
		ClientData old=Tcl_GetHashValue(entryPtr);
		if (old==data) {
			/* nothing to do! This data is already registered with that name. */
			return TCL_OK;
		} else if (mode==REG_VAR_DELETE_OLD) {
			statePtr->deleteProc(old);
		}
	}
	Tcl_SetHashValue(entryPtr,(ClientData)data);
	return TCL_OK;
}

int getVarFromObj(ClientData clientData, Tcl_Interp *interp, Tcl_Obj *CONST name,
		void **iPtrPtr)
{
	Tcl_HashEntry *entryPtr=NULL;
	StateManager_t statePtr=(StateManager_t)clientData;
	void *iPtr=NULL;
	if (clientData==NULL) {
		Tcl_AppendResult(interp,"clientData was NULL",NULL);
		return TCL_ERROR;
	}
	if (name==NULL) {
		Tcl_AppendResult(interp,"name was NULL",NULL);
		return TCL_ERROR;
	}
	if (iPtrPtr==NULL) {
		Tcl_AppendResult(interp,"iPtrPtr was NULL",NULL);
		return TCL_ERROR;
	}

	entryPtr=Tcl_FindHashEntry(&statePtr->hash,Tcl_GetString(name));
	if (entryPtr==NULL) {
		Tcl_AppendResult(interp,"Unknown var: ", Tcl_GetString(name),NULL);
		return TCL_ERROR;
	}
	iPtr=(void*)Tcl_GetHashValue(entryPtr);
	*iPtrPtr=iPtr;
	return TCL_OK;
}

int getVarFromObjKey(const char *state_key, Tcl_Interp *interp, Tcl_Obj *CONST name,
		void **iPtrPtr)
{
	StateManager_t statePtr=(StateManager_t)Tcl_GetAssocData(interp,state_key,NULL);
	if (statePtr==NULL) {
		Tcl_AppendResult(interp,"No state stored by key `",state_key,"'\n",NULL);
		return TCL_ERROR;
	}
	return getVarFromObj(statePtr,interp,name,iPtrPtr);
}

/* StateManagerCmd --
 * This implements the StateManager command, which has these subcommands:
 * 	names ?pattern?
 * 	exists ?name?
 * 	delete name
 *
 * Results:
 *  A standard Tcl command result.
 */
int StateManagerCmd(ClientData clientData, Tcl_Interp *interp,
		int objc, Tcl_Obj *CONST objv[])
{
	StateManager_t statePtr=(StateManager_t)clientData;
	void *iPtr=NULL;
	Tcl_HashEntry *entryPtr=NULL;
	Tcl_Obj *valueObjPtr=NULL;

	/* the subCmd array defines the allowed values for
	 * the subcommand.
	 */
	int index=-1;
	CONST char *subCmds[] = {
		"delete","exists","names",NULL};
	enum StateCmdIx {
		DeleteIx, ExistsIx, NamesIx};

	if (objc<=1) {
		Tcl_WrongNumArgs(interp,1,objv,"option ?arg ...?");
		return TCL_ERROR;
	}

	if (Tcl_GetIndexFromObj(interp,objv[1],subCmds,"option",TCL_EXACT,&index)!=TCL_OK)
	{
		return statePtr->unknownCmd(clientData,interp,objc,objv);
	}

	switch (index) {
		case ExistsIx:
			if (objc!=3) goto err;
			return varExistsTcl(interp,statePtr,objv[2]);
			break;
		case NamesIx:
			if (objc>2) goto err;
			return varNames(interp,statePtr);
			break;
		case DeleteIx:
			if (objc!=3) goto err;
			return varDelete0(interp,statePtr, objv[2]);
			break;
		default:
			return statePtr->unknownCmd(statePtr,interp,objc,objv);
			break;
	}
	return TCL_OK;
err:
	Tcl_WrongNumArgs(interp,1,objv,"option ?arg ...?");
	return TCL_ERROR;
}

/* varDelete0 --
 * Remove a variable from the state manager and free its resources.
 * Results:
 *  A standard Tcl command result.
 */
int varDelete0(Tcl_Interp *interp, StateManager_t statePtr,
		Tcl_Obj *objName)
{
	void *iPtr=NULL;
	Tcl_HashEntry *entryPtr=NULL;
	entryPtr=Tcl_FindHashEntry(&statePtr->hash,Tcl_GetString(objName));
	if (entryPtr==NULL) {
		Tcl_AppendResult(interp,"Unknown var: ",
				Tcl_GetString(objName),NULL);
		return TCL_ERROR;
	}
	iPtr=Tcl_GetHashValue(entryPtr);
	statePtr->deleteProc(iPtr);
	Tcl_DeleteHashEntry(entryPtr);
	return TCL_OK;
}

/* function to initialize state for a variable type */
int InitializeStateManager(Tcl_Interp *interp, const char *key,
		const char *cmd_name,
		int (*unknownCmd)(ClientData,Tcl_Interp*,int,Tcl_Obj *CONST objv[]),
		void (*deleteProc)(void *ptr))
{
	StateManager_t state=NULL;
	if (NULL!=Tcl_GetAssocData(interp,key,NULL)) return TCL_OK;
	/* otherwise, we need to create a new context and associate it with
	 * the Tcl interpreter.
	 */
	state=(StateManager_t)ckalloc(sizeof(struct StateManager_s));
	Tcl_InitHashTable(&state->hash,TCL_STRING_KEYS);
	Tcl_SetAssocData(interp,key,NULL,(ClientData)state);
	state->uid=0;
	state->deleteProc=deleteProc;
	state->unknownCmd=unknownCmd;
	state->max_num_reg_types=100;
	state->num_reg_types=0;
	state->reg_type_names=(char**)ckalloc(101*sizeof(char*));
	memset(state->reg_type_names,0,101*sizeof(char*));
	state->reg_types_create_procs=(CreateObjFunc*)ckalloc(100*sizeof(CreateObjFunc));
	state->reg_types_instance_commands=(InstanceCommandFunc*)ckalloc(100*sizeof(InstanceCommandFunc));
	Tcl_CreateObjCommand(interp,cmd_name, StateManagerCmd, (ClientData)state,StateManagerDeleteProc);
	int len=strlen(cmd_name);
	state->prefix=(char*)ckalloc(len+6);
	sprintf(state->prefix,"%s%s",cmd_name,"#%04d");
	return TCL_OK;
}
