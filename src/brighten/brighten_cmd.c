/*
 * This file is part of MVTH - the Machine Vision Test Harness.
 *
 * Brighten an image.
 *
 * Copyright (C) 2003,2004 Samuel P. Bromley <sam@sambromley.com>
 *
 * This file is part of MVTH - the Machine Vision Test Harness.
 *
 * MVTH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License Version 3,
 * as published by the Free Software Foundation.
 *
 * MVTH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * (see the file named "COPYING"), and a copy of the GNU Lesser General
 * Public License (see the file named "COPYING.LESSER") along with MVTH.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcl.h>
#include <assert.h>
#include "dynamic_symbols.h"
#include "base/mvthimagestate.h"

int brighten_cmd(ClientData clientData, Tcl_Interp *interp,
		int objc, Tcl_Obj *CONST objv[])
{
	char *name=NULL;
	int namelen;
	double factor;
	image_t *img=NULL;
	MvthImage *mimg=NULL;

	if (objc!=3)
	{
		Tcl_WrongNumArgs(interp,1,objv,"?name? ?factor?");
		return TCL_ERROR;
	}

	name=Tcl_GetStringFromObj(objv[1],&namelen);
	if (Tcl_GetDoubleFromObj(interp,objv[2],&factor)==TCL_ERROR)
		return TCL_ERROR;

	if (getMvthImageFromObj(interp,objv[1],&mimg)!=TCL_OK) return TCL_ERROR;

	img=mimg->img;
	//register_image_undo_var(name);

	/* do the filter */
	assert(brighten_fltr!=NULL);
	brighten_fltr(img,factor);
	stamp_image_t(img);
	
	return TCL_OK;
}
