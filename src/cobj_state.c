/*
 * This file is part of the TclStateManager module.
 *
 * Maintain handles to pointers to C-objects. 
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
 * 
 */
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <tcl.h>
#include "variable_state.h"
#include "cobj_state.h"

#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif

#define COBJSTATEKEY "cobjstate"

// Forward declarations
int  cObjCmd(ClientData data, Tcl_Interp *interp, int objc, 
		Tcl_Obj *CONST objv[]);
int  cObjCreate(ClientData data, Tcl_Interp *interp, int objc, 
		Tcl_Obj *CONST objv[]);
void cObjDelete(void *ptr);

int getcObjFromObj(Tcl_Interp *interp, Tcl_Obj *CONST name,
						const char *type_name,
		        cObj **iPtrPtr)
{
	cObj *obj=NULL;
	if (type_name==NULL) {
		Tcl_AppendResult(interp,"invalid type passed to ",__func__," \n",NULL);
		return TCL_ERROR;
	}
	if (getVarFromObjKey(COBJSTATEKEY,interp,name,(void**)iPtrPtr)!=TCL_OK)
		return TCL_ERROR;
	obj=*iPtrPtr;
	if (obj->type_hash != TYPEHASH(type_name,-1)) {
		Tcl_AppendResult(interp,"src object is not of type ",type_name,"\n", NULL);
		*iPtrPtr=NULL;
		return TCL_ERROR;
	}
	return TCL_OK;
}

// The following creates and initialize an cObj objects
int cObjState_Init(Tcl_Interp *interp)
{
	InitializeStateManager(interp,COBJSTATEKEY,"cobj",cObjCmd,cObjDelete);
	return TCL_OK;
}

/* cObjCmd --
 * This implements the cObj command, which has these subcommands:
 *  create <type> 
 *   where "type" must be the name of one of the registered object types,
 *   for example "cam", or "img", or perhaps "mycoolstruct"
 *
 * Results:
 *  A standard Tcl command result.
 */
int cObjCmd(ClientData data, Tcl_Interp *interp, 
		int objc, Tcl_Obj *CONST objv[])
{
	// the subCmd array defines the allowed values for the subcommand.  
	CONST char *subCmds[] = {
		"create",NULL};
	enum cObjIx { CreateIx };

	if (objc<3) {
		Tcl_WrongNumArgs(interp,1,objv,"[sub-command] [type] <args>");
		return TCL_ERROR;
	}

	int index;
	if (Tcl_GetIndexFromObj(interp,objv[1],subCmds,"subcommand",0,&index)!=TCL_OK) {
		return TCL_ERROR;
	}

	// reset the result so that we don't have the error from StateManagerCmd 
	Tcl_ResetResult(interp);

	switch (index) {
		case CreateIx:
			return cObjCreate(data,interp,objc,objv);
			break;
		default:
			return TCL_ERROR;
	}
	return TCL_ERROR;
}

int registerNewType(Tcl_Interp *interp,
		const char *type_name,
		CreateObjFunc createObjFunc,
		InstanceCommandFunc instanceCommand)
{
	if (type_name == NULL || createObjFunc == NULL || instanceCommand == NULL)
	{
		return TCL_ERROR;
	}

	StateManager_t statePtr=(StateManager_t)Tcl_GetAssocData(interp,COBJSTATEKEY,NULL);
	if (statePtr==NULL) {
		Tcl_AppendResult(interp,"No state stored by key `",COBJSTATEKEY,"'\n",NULL);
		return TCL_ERROR;
	}

	if (statePtr->num_reg_types >= statePtr->max_num_reg_types) {
		Tcl_AppendResult(interp,"No slots left to register new type `",
				type_name,
				"'\n",NULL);
		return TCL_ERROR;
	}
	if (statePtr->num_reg_types==0) {
		/* this is a way to ensure that reg_type_names is initialied */
		int i;
		for (i=0;i<statePtr->max_num_reg_types;i++) {
			statePtr->reg_type_names[i]=NULL;
			statePtr->reg_types_create_procs[i]=NULL;
			statePtr->reg_types_instance_commands[i]=NULL;
		}
		statePtr->reg_type_names[statePtr->max_num_reg_types]=NULL;
	}
	statePtr->reg_types_create_procs[statePtr->num_reg_types]=createObjFunc;
	statePtr->reg_types_instance_commands[statePtr->num_reg_types]=instanceCommand;
	statePtr->reg_type_names[statePtr->num_reg_types]=strdup(type_name);
	statePtr->reg_type_names[statePtr->num_reg_types+1]=NULL;
	statePtr->num_reg_types++;
	return TCL_OK;
}

/* cObjInstanceCmd --
 * This implements the command tied to each instance of a
 * cObj Object. It looks at the Object type and passes control to the
 * instance command of the appropriate type.
 *
 * Results:
 *  A standard Tcl command result.
 */
int cObjInstanceCmd(ClientData data, Tcl_Interp *interp,
		          int objc, Tcl_Obj *CONST objv[])
{
	if (data==NULL) return TCL_ERROR;
	if (objc==1) return TCL_OK; // No arguments were passed
	ObjCmdClientData *cdata=(ObjCmdClientData*)data;
	CONST char *subCmds[] = {"type",NULL};
	enum cmdIx {TypeIx};
	int index;
	if (Tcl_GetIndexFromObj(interp,objv[1],subCmds,"subcommand",0,&index)!=TCL_OK)
	{
		/* then we did not recognize the subcommand. Perhaps the
		 * specific type commands will understand it? */
		/* clear the error */
		Tcl_ResetResult(interp);
		// Hand control to object-specfic instance command
		return (*cdata->instanceCommand)(data,interp,objc,objv);
	}

	// Are we asked to report object type?
	switch(index) {
		case TypeIx:
			Tcl_AppendResult(interp,cdata->mSelf->type_name,NULL);
			return TCL_OK;
	}
	return TCL_OK;
}

// The following routine actually creates cObj Objects 
int cObjCreate(ClientData data, Tcl_Interp *interp,
		int objc, Tcl_Obj *CONST objv[])
{
	StateManager_t statePtr=(StateManager_t)data;
	cObj *oPtr;
	ObjCmdClientData *cdata=NULL;
	char *name_ptr=NULL;
	char name[20];

	// Get a variable name
	if (varUniqName(interp,statePtr,name)!=TCL_OK) return TCL_ERROR;
	name_ptr=name;

	// Ceate object of the specified type
	int index;
	if (Tcl_GetIndexFromObj(interp,objv[2],statePtr->reg_type_names,"type",0,&index)!=TCL_OK)
		return TCL_ERROR;
	oPtr=(cObj*)ckalloc(sizeof(cObj));
	memset(oPtr,0,sizeof(cObj));
	if (statePtr->reg_types_create_procs[index](data,interp,objc,objv,oPtr)!=TCL_OK) {
		ckfree(oPtr);
		return TCL_ERROR;
	}
	// Register it
	registerVar(interp,statePtr,(ClientData)oPtr,name_ptr,REG_VAR_DELETE_OLD);
	// make a command with the same name as this object 
	cdata=(ObjCmdClientData*)ckalloc(sizeof(ObjCmdClientData));
	memset(cdata,0,sizeof(ObjCmdClientData));
	cdata->state=statePtr;
	cdata->mSelf=oPtr;
	cdata->instanceCommand=statePtr->reg_types_instance_commands[index];
	Tcl_CreateObjCommand(interp,name_ptr,cObjInstanceCmd,(ClientData)cdata,NULL);

	Tcl_AppendResult(interp,name_ptr,NULL);
	return TCL_OK;
}

void cObjDelete(void *ptr)
{
	cObj *oPtr=(cObj *)ptr;
	if (oPtr==NULL) return;
	if (oPtr->deleteFunc!=NULL) oPtr->deleteFunc(oPtr->object);
	ckfree((char*)oPtr);
	return;
}

/* Hash a string to an integer using the FNV1a Hashing algorithm */
uint64_t FNV1aHash(const char *str, int maxlen) {
	int i;
	uint64_t hash;
	uint64_t FNV_offset_basis = 14695981039346656037U;
	uint64_t FNV_Prime = 1099511628211U;
	hash = FNV_offset_basis;
	for (i=0;(i<maxlen || maxlen==-1) && *str!='\0';i++, str++) {
		hash = hash ^ (uint64_t)(*str);
		hash*=FNV_Prime;
	}
	return hash;
}
