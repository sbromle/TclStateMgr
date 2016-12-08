#Look for glib2.0. If found, defines:
# GTK_glib_INLUCDE_PATH
# GTK_glib_LIBRARY
# amoung others.
include  (FindTclStub)
option (USE_TCL_STUBS "Enable use of TCL stubs library")

if (USE_TCL_STUBS)
ADD_DEFINITIONS("-DUSE_TCL_STUBS")
endif(USE_TCL_STUBS)

include_directories(${TCL_INCLUDE_PATH})
include_directories(.)

########### next target ###############
set (libstatemgr_SRCS
	variable_state.c
	variable_state.h
	cobj_state.c
	cobj_state.h
)

#MSVC needs static .lib files to work properly
IF(WIN32 OR MSVC)
	add_library(statemgr SHARED ${libstatemgr_SRCS})
else(WIN32 OR MSVC)
	add_library(statemgr SHARED ${libstatemgr_SRCS})
ENDIF(WIN32 OR MSVC)

if (USE_TCL_STUBS)
	target_link_libraries(statemgr ${TCL_STUB_LIBRARY})
else (USE_TCL_STUBS)
	target_link_libraries(statemgr ${TCL_LIBRARY})
endif (USE_TCL_STUBS)

set_target_properties (statemgr PROPERTIES VERSION 1.0 SOVERSION 1 INSTALL_RPATH_USE_LINK_PATH on INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

########### install files ###############

# Headers 
install(FILES variable_state.h cobj_state.h helperwrapper.hpp DESTINATION include/${PACKAGE})
# Targets 
install(TARGETS statemgr DESTINATION lib COMPONENT libraries)
