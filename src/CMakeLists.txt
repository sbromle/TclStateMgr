#Look for glib2.0. If found, defines:
# GTK_glib_INLUCDE_PATH
# GTK_glib_LIBRARY
# amoung others.
include  (FindTclStub)
option (USE_TCL_STUBS "Enable use of TCL stubs library")

include_directories(${TCL_INCLUDE_PATH})
include_directories(.)

########### next target ###############
set (libstatemgr_SRCS
	variable_state.c
	variable_state.h
	cobj_state.c
	cobj_state.h
	cobj_args.cpp
	cobj_args.hpp
)

add_library(statemgr SHARED ${libstatemgr_SRCS})

if (USE_TCL_STUBS)
	target_link_libraries(statemgr ${TCL_STUB_LIBRARY})
else (USE_TCL_STUBS)
	target_link_libraries(statemgr ${TCL_LIBRARY})
endif (USE_TCL_STUBS)

set_target_properties (statemgr PROPERTIES VERSION 1.0 SOVERSION 1 INSTALL_RPATH_USE_LINK_PATH on INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

########### install files ###############

# Headers 
install(FILES variable_state.h cobj_state.h helperwrapper.hpp cobj_args.hpp DESTINATION include/${PACKAGE})
# Targets 
install(TARGETS statemgr DESTINATION lib)

