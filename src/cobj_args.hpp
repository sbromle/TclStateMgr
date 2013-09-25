/*
 * This file is part of TclStateManager module.
 *
 * Provides C++ variadic template for getting arguments from Tcl
 * Declare the variables, and call
 * Tcl_GetArgsFromObjs(interp, objc, objv, &obj1, &obj2, &obj3, ...etc)
 * Write template specializations to Tcl_GetArgFromObj to support custom types.
 *
 * Copyright (C) 2013 Whitecap Scientific Corporation
 * Original author: Michael Barriault <mike.barriault@whitecapscientific.com>
 *
 * TclStateManager is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License Version 3,
 * as published by the Free Software Foundation.
 *
 * TclStateManager is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * (see the file named "COPYING"), and a copy of the GNU Lesser General
 * Public License (see the file named "COPYING.LESSER") along with
 * TclStateManager. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STATE_ARGS_H
#define STATE_ARGS_H

#include <tcl.h>

template<typename T>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, T* ptr) {
	Tcl_SetResult(interp, (char*)"Type not implemented.\n", NULL);
	return TCL_ERROR;
}

// GetArgFromObj predefined types
//
// NOTE ON char USAGE
// char is often used for both 8-bit numbers and ASCII characters.
// Sending a string of length one is treated as an ASCII character.
// Sending a string longer than one character is treated as a number.
// To pass single-digit numbers, prefix with 0.
//
// Define your own template specialization to add new types. Return should be TCL_OK
// or TCL_ERROR depending on success.
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, char* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, unsigned char* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, signed char* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, unsigned short* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, short* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, unsigned int* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, int* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, unsigned long* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, long* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, float* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, double* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, long double* ptr);
template<>
int Tcl_GetArgFromObj(Tcl_Interp* interp, Tcl_Obj *obj, char** ptr);

int Tcl_GetArgsFromObjs(Tcl_Interp* interp, int i, int objc, Tcl_Obj *CONST objv[]);

template<typename T, typename... Args>
int Tcl_GetArgsFromObjs(Tcl_Interp* interp, int i, int objc, Tcl_Obj *CONST objv[], T* ptr, Args... args) {
	if ( i >= objc ) {
		Tcl_SetResult(interp, (char*)"Not enough arguments.", NULL);
		return TCL_ERROR;
	}
	else if ( Tcl_GetArgFromObj(interp, *objv, ptr) != TCL_OK ) {
		return TCL_ERROR;
	}
	else {
		// This bit here is to prevent objv pointer from being incremented too far
		++i;
		if ( i < objc-1 )
			++objv;
		else // We're at the end of known objects anyway, increment i one extra time in case there are more args
			++i;
		return Tcl_GetArgsFromObjs(interp, i, objc, objv, args...);
	}
}

template<typename T, typename... Args>
int Tcl_GetArgsFromObjs(Tcl_Interp* interp, int objc, Tcl_Obj *CONST objv[], T* ptr, Args... args) {
	return Tcl_GetArgsFromObjs(interp, 0, objc, objv, ptr, args...);
}

#endif
