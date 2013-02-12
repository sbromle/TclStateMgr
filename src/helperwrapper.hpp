/*
 * This file is part of TclStateManager module.
 *
 * Provides C++ template wrappers for storing complex objects in cObj.
 *
 * Copyright (C) 2011 Whitecap Scientific Corporation
 * Original author: Michael Barriault <mike.barriault@whitecapscientific.com>
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

#ifndef HELP_WRAPPER_H
#define HELP_WRAPPER_H

#include "cobj_state.h"

// Base wrapper class, just contains a pointer
template<typename T>
struct wrapper {
    T* ptr;
};

// Makes a wrapper around an instance
template<typename T>
wrapper<T>* makeWrapper(T* ptr) {
    wrapper<T>* wpr = new wrapper<T>;
    wpr->ptr = ptr;
    return wpr;
}

// Makes a wrapper around an instance of a child class
template<typename T, typename U>
wrapper<T>* makeWrapper(U* ptr) {
    wrapper<T>* wpr = new wrapper<T>;
    wpr->ptr = static_cast<T*>(ptr);
    return wpr;
}

// Gets the instance from a wrapper stored in cObj-compliant ClientData
template<typename T>
T* getFrom(ClientData data) {
    ObjCmdClientData* cdata = (ObjCmdClientData*)data;
    wrapper<T>* wpr = reinterpret_cast<wrapper<T>*>(cdata->mSelf->object);
    return wpr->ptr;
}

// Gets the instance as a child class
template<typename T, typename U>
U* getFrom(ClientData data) {
    ObjCmdClientData* cdata = (ObjCmdClientData*)data;
    wrapper<T>* wpr = reinterpret_cast<wrapper<T>*>(cdata->mSelf->object);
    return dynamic_cast<U*>(wpr->ptr);
}

// Handy deleter lambda
typedef void (*deleterFunc)(void*);
template<typename T>
deleterFunc deleter() {
    return [](void* ptr) { delete (T*)ptr; };
}

#endif
