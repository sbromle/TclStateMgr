/*
 * This file is part of MVTH - the Machine Vision Test Harness.
 *
 * Convert a grayscale image to rgb.
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
#include "dynamic_symbols_mvth.h"
#include "base/mvthimagestate.h"

int rgb_cmd(ClientData clientData, Tcl_Interp *interp,
		int objc, Tcl_Obj *CONST objv[])
{
	image_t *img=NULL;
	MvthImage *mimg=NULL;

	if (objc!=2)
	{
		Tcl_WrongNumArgs(interp,1,objv,"?name?");
		return TCL_ERROR;
	}

	if (getMvthImageFromObj(interp,objv[1],&mimg)!=TCL_OK) return TCL_ERROR;

	img=mimg->img;

	/* register with the undo substructure */
	//register_image_undo_var(name);

	if (img->bands!=1)
	{
		Tcl_AppendResult(interp,"Image is not grayscale.",NULL);
		return TCL_ERROR;
	}

	assert(grayscale2rgb_fltr!=NULL);
	img=grayscale2rgb_fltr(img);
	mvthImageReplace(img,mimg);
	//register_image_var(img,name);
	//stamp_image_t(img);

	return TCL_OK;
}
