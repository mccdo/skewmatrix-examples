# This is part of the Findosg* suite used to find OpenSceneGraph components.
# Each component is separate and you must opt in to each module. You must 
# also opt into OpenGL and OpenThreads (and Producer if needed) as these 
# modules won't do it for you. This is to allow you control over your own 
# system piece by piece in case you need to opt out of certain components
# or change the Find behavior for a particular module (perhaps because the
# default FindOpenGL.cmake module doesn't work with your system as an
# example).
# If you want to use a more convenient module that includes everything,
# use the FindOpenSceneGraph.cmake instead of the Findosg*.cmake modules.
# 
# Locate osg
# This module defines
# OSG_LIBRARY
# OSG_FOUND, if false, do not try to link to osg
# OSG_INCLUDE_DIR, where to find the headers
#
# $OSGDIR is an environment variable that would
# correspond to the ./configure --prefix=$OSGDIR
# used in building osg.
#
# Created by Eric Wing.

# Header files are presumed to be included like
# #include <osg/PositionAttitudeTransform>
# #include <osgUtil/SceneView>

# For Windows, I have attempted to incorporate the environmental variables
# and registry entries used by Mike E. Weiblen's (mew) OSG installer. 
# 
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# the library to override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.
#
# I originally had implemented some really nasty hacks to do OS X
# framework detection. CMake per my request has introduced native support
# for this so the code has been simplified. But for this to work,
# you must be using the new CMake code (introduced just before Jan 1st, 2006).

FIND_PATH(OSG_INCLUDE_DIR osg/PositionAttitudeTransform
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    "C:/Program Files/OpenSceneGraph/include"
    "C:/Program Files (x86)/OpenSceneGraph/include"
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    ~/Library/Frameworks
    /Library/Frameworks
)

MACRO(FIND_OSG_LIBRARY MYLIBRARY MYLIBRARYNAME)

    FIND_LIBRARY(${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
        PATHS
        "C:/Program Files/OpenSceneGraph/lib"
        "C:/Program Files (x86)/OpenSceneGraph/lib"
        $ENV{OSG_DIR}/lib
        $ENV{OSG_DIR}/Build/lib
        $ENV{OSG_DIR}
        $ENV{OSGDIR}/lib
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}/lib
        $ENV{OSG_ROOT}/Build/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )

ENDMACRO(FIND_OSG_LIBRARY LIBRARY LIBRARYNAME)

FIND_OSG_LIBRARY(OSG_LIBRARY osg)
FIND_OSG_LIBRARY(OSGUTIL_LIBRARY osgUtil)
FIND_OSG_LIBRARY(OSGDB_LIBRARY osgDB)
FIND_OSG_LIBRARY(OSGTEXT_LIBRARY osgText)
FIND_OSG_LIBRARY(OSGTERRAIN_LIBRARY osgTerrain)
FIND_OSG_LIBRARY(OSGFX_LIBRARY osgFX)
FIND_OSG_LIBRARY(OSGSIM_LIBRARY osgSim)
FIND_OSG_LIBRARY(OSGVIEWER_LIBRARY osgViewer)

SET(OSG_FOUND "NO")
IF(OSG_LIBRARY AND OSG_INCLUDE_DIR)
    SET(OSG_FOUND "YES")

    # OSG_LIBRARY should contain the absolute path to the core OSG library.
    # Set OSG_LIBRARIES dir to the directory containing this lib.
    GET_FILENAME_COMPONENT(OSG_LIBRARIES_DIR ${OSG_LIBRARY} PATH)

ENDIF(OSG_LIBRARY AND OSG_INCLUDE_DIR)

