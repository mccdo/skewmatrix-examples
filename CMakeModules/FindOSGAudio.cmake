# Locate osgAudio.
#
# This script defines:
#   OSGAUDIO_FOUND, set to 1 if found
#   OSGAUDIO_LIBRARIES -- all libs, if more than one
#   OSGAUDIO_LIBRARY -- the main osgAudio library.
#   OSGAUDIO_INCLUDE_DIR
#
# This script will look in standard locations for installed osgAudio. However, if you
# install osgAudio into a non-standard location, you can use the OSGAUDIO_ROOT
# variable (in environment or CMake) to specify the location.
#
# You can also use osgAudio out of a source tree by specifying OSGAUDIO_SOURCE_DIR
# and OSGAUDIO_BUILD_DIR (in environment or CMake).


SET( OSGAUDIO_BUILD_DIR "" CACHE PATH "If using osgAudio out of a source tree, specify the build directory." )
SET( OSGAUDIO_SOURCE_DIR "" CACHE PATH "If using osgAudio out of a source tree, specify the root of the source tree." )
SET( OSGAUDIO_ROOT "" CACHE PATH "Specify non-standard osgAudio install directory. It is the parent of the include and lib dirs." )

MACRO( FIND_OSGAUDIO_INCLUDE THIS_OSGAUDIO_INCLUDE_DIR THIS_OSGAUDIO_INCLUDE_FILE )
    UNSET( ${THIS_OSGAUDIO_INCLUDE_DIR} CACHE )
    MARK_AS_ADVANCED( ${THIS_OSGAUDIO_INCLUDE_DIR} )
    FIND_PATH( ${THIS_OSGAUDIO_INCLUDE_DIR} ${THIS_OSGAUDIO_INCLUDE_FILE}
        PATHS
            ${OSGAUDIO_ROOT}
            $ENV{OSGAUDIO_ROOT}
            ${OSGAUDIO_SOURCE_DIR}
            $ENV{OSGAUDIO_SOURCE_DIR}
            /usr/local
            /usr
            /sw/ # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt
            "C:/Program Files/osgAudio"
            "C:/Program Files (x86)/osgAudio"
            ~/Library/Frameworks
            /Library/Frameworks
        PATH_SUFFIXES
            include
            .
    )
ENDMACRO( FIND_OSGAUDIO_INCLUDE THIS_OSGAUDIO_INCLUDE_DIR THIS_OSGAUDIO_INCLUDE_FILE )

FIND_OSGAUDIO_INCLUDE( OSGAUDIO_INCLUDE_DIR osgAudio/SoundNode )
# message( STATUS ${OSGAUDIO_INCLUDE_DIR} )

MACRO( FIND_OSGAUDIO_LIBRARY MYLIBRARY MYLIBRARYNAME )
    UNSET( ${MYLIBRARY} CACHE )
    UNSET( ${MYLIBRARY}_debug CACHE )
    MARK_AS_ADVANCED( ${MYLIBRARY} )
    MARK_AS_ADVANCED( ${MYLIBRARY}_debug )
    FIND_LIBRARY( ${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
        PATHS
            ${OSGAUDIO_ROOT}
            $ENV{OSGAUDIO_ROOT}
            ${OSGAUDIO_BUILD_DIR}
            $ENV{OSGAUDIO_BUILD_DIR}
            ~/Library/Frameworks
            /Library/Frameworks
            /usr/local
            /usr
            /sw
            /opt/local
            /opt/csw
            /opt
            "C:/Program Files/osgAudio"
            "C:/Program Files (x86)/osgAudio"
            /usr/freeware/lib64
        PATH_SUFFIXES
            lib
            .
    )
    FIND_LIBRARY( ${MYLIBRARY}_debug
        NAMES ${MYLIBRARYNAME}d
        PATHS
            ${OSGAUDIO_ROOT}
            $ENV{OSGAUDIO_ROOT}
            ${OSGAUDIO_BUILD_DIR}
            $ENV{OSGAUDIO_BUILD_DIR}
            ~/Library/Frameworks
            /Library/Frameworks
            /usr/local
            /usr
            /sw
            /opt/local
            /opt/csw
            /opt
            "C:/Program Files/osgAudio"
            "C:/Program Files (x86)/osgAudio"
            /usr/freeware/lib64
        PATH_SUFFIXES
            lib
            .
    )
#    message( STATUS ${${MYLIBRARY}} ${${MYLIBRARY}_debug} )
#    message( STATUS ${MYLIBRARYNAME} )
    IF( ${MYLIBRARY} )
        SET( OSGAUDIO_LIBRARIES ${OSGAUDIO_LIBRARIES}
            "optimized" ${${MYLIBRARY}}
        )
    ENDIF( ${MYLIBRARY} )
    IF( ${MYLIBRARY}_debug )
        SET( OSGAUDIO_LIBRARIES ${OSGAUDIO_LIBRARIES}
            "debug" ${${MYLIBRARY}_debug}
        )
    ENDIF( ${MYLIBRARY}_debug )
ENDMACRO(FIND_OSGAUDIO_LIBRARY LIBRARY LIBRARYNAME)

FIND_OSGAUDIO_LIBRARY( OSGAUDIO_LIBRARY osgAudio )

SET( OSGAUDIO_FOUND 0 )
IF( OSGAUDIO_LIBRARIES AND OSGAUDIO_INCLUDE_DIR )
    SET( OSGAUDIO_FOUND 1 )
ENDIF( OSGAUDIO_LIBRARIES AND OSGAUDIO_INCLUDE_DIR )
