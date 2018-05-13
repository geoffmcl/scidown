
# CMakeLists.txt, generated gencmake.pl, on 2018/05/11 13:21:10
cmake_minimum_required( VERSION 2.8.8 )

# CMakeScripts or use the ones that come by default with CMake.
# set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

project( scidown )

# The version number.
set( scidown_MAJOR 3 )
set( scidown_MINOR 0 )
set( scidown_POINT 0 )

# Allow developer to select is Dynamic or static library built
set( LIB_TYPE STATIC )  # set default static
option( BUILD_SHARED_LIB "Set ON to build shared Library (DLL)" OFF )

if(CMAKE_COMPILER_IS_GNUCXX)
    set( WARNING_FLAGS -Wall )
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
   set( WARNING_FLAGS "-Wall -Wno-overloaded-virtual" )
endif() 

if(WIN32 AND MSVC)
    # turn off various warnings - none needed in this compile
    set(WARNING_FLAGS "${WARNING_FLAGS} /wd4996")
    # foreach(warning 4244 4251 4267 4275 4290 4786 4305)
    #     set(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
    # endforeach(warning)
    set( MSVC_FLAGS "-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS" )
    # if (${MSVC_VERSION} EQUAL 1600)
    #    set( MSVC_LD_FLAGS "/FORCE:MULTIPLE" )
    # endif (${MSVC_VERSION} EQUAL 1600)
    #set( NOMINMAX 1 )
    # to distinguish between debug and release libs
    set( CMAKE_DEBUG_POSTFIX "d" )
else()
    # items for unix
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}" )

# configuration file, if needed
# configure_file( ${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h )
# add_definitions( -DHAVE_CONFIG_H )
# include_directories( ${CMAKE_BINARY_DIR} )

if(BUILD_SHARED_LIB)
   set(LIB_TYPE SHARED)
   message(STATUS "*** Building DLL library ${LIB_TYPE}")
else()
   message(STATUS "*** Option BUILD_SHARED_LIB is OFF ${LIB_TYPE}")
endif()
#################################################
##### LIBRARIES #####
#################################################

# src LIBRARY from [src],
# have 12 C/C++ sources, 10 headers
set(name hoedown)
set(dir src)
include_directories( ${dir} )
set(${name}_SRCS
    ${dir}/autolink.c
    ${dir}/buffer.c
    ${dir}/constants.c
    ${dir}/document.c
    ${dir}/escape.c
    ${dir}/html.c
    ${dir}/html_blocks.c
    ${dir}/html_smartypants.c
    ${dir}/latex.c
    ${dir}/stack.c
    ${dir}/utils.c
    ${dir}/version.c
    )
set(${name}_HDRS
    ${dir}/autolink.h
    ${dir}/buffer.h
    ${dir}/constants.h
    ${dir}/document.h
    ${dir}/escape.h
    ${dir}/html.h
    ${dir}/latex.h
    ${dir}/stack.h
    ${dir}/utils.h
    ${dir}/version.h
    )
add_library( ${name} ${LIB_TYPE} ${${name}_SRCS} ${${name}_HDRS} )
list(APPEND add_LIBS ${name})
# deal with install, if any...
install( TARGETS ${name}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib )
install(FILES ${${name}_HDRS} DESTINATION include/hoedown)

add_definitions( -DADD_CHARTER )
set(name charter)
set(dir src/charter/src)
set(${name}_SRCS
    ${dir}/charter_string.c
    ${dir}/clist.c
    ${dir}/parser.c
    ${dir}/charter.c
    ${dir}/latex.c
    ${dir}/svg.c
    ${dir}/svg_utils.c
    ${dir}/tinyexpr/tinyexpr.c
    ${dir}/csv_parser/csvparser.c
    )
set(${name}_HDRS
    ${dir}/charter_string.h
    ${dir}/clist.h
    ${dir}/parser.h
    ${dir}/charter.h
    ${dir}/svg_utils.h
    ${dir}/tinyexpr/tinyexpr.h
    ${dir}/csv_parser/csvparser.h
    )
add_library( ${name} ${${name}_SRCS} ${${name}_HDRS} )
list(APPEND add_LIBS ${name})
# deal with install, if any...
#install( TARGETS ${name}
#    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib )
#install(FILES ${${name}_HDRS} DESTINATION include)

#################################################
##### EXECUTABLES #####
#################################################

# bin EXECUTABLE from [bin],
# have 2 C/C++ sources, 1 headers
set(name scidown)
set(dir bin)
set(${name}_SRCS
    ${dir}/scidown.c
    )
set(${name}_HDRS
    ${dir}/common.h
    )
add_executable( ${name} ${${name}_SRCS} ${${name}_HDRS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

set(name smartypants)
set(dir bin)
set(${name}_SRCS
    ${dir}/smartypants.c
    )
set(${name}_HDRS
    ${dir}/common.h
    )
add_executable( ${name} ${${name}_SRCS} ) # ${${name}_HDRS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

set(name md2html)
set(dir bin)
set(${name}_SRCS
    ${dir}/${name}.cxx
    )
add_executable( ${name} ${${name}_SRCS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

# eof
