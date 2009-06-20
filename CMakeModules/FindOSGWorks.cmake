# Locate OSGWorks
# This module defines:
#   OSGWORKS_FOUND, set to "YES" or "NO".   
#   OSGWORKS_LIBRARIES
#   OSGWORKS_INCLUDE_DIR

MACRO( FIND_OSGWORKS_INCLUDE THIS_OSGWORKS_INCLUDE_DIR THIS_OSGWORKS_INCLUDE_FILE )
    FIND_PATH( ${THIS_OSGWORKS_INCLUDE_DIR} ${THIS_OSGWORKS_INCLUDE_FILE}
        PATHS
            $ENV{OSGWORKS_SOURCE_DIR}
            /usr/local/
            /usr/
            /sw/ # Fink
            /opt/local/ # DarwinPorts
            /opt/csw/ # Blastwave
            /opt/
            "C:/Program Files/OSGWorks"
            "C:/Program Files (x86)/OSGWorks"
            [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manage
    r\\Environment;OSGWORKS_ROOT]/
            ~/Library/Frameworks
            /Library/Frameworks
        PATH_SUFFIXES
            include
    )
ENDMACRO( FIND_OSGWORKS_INCLUDE THIS_OSGWORKS_INCLUDE_DIR THIS_OSGWORKS_INCLUDE_FILE )

FIND_OSGWORKS_INCLUDE( OSGWORKS_INCLUDE_DIR osgTools/FindNamedNode.h )
# message( STATUS ${OSGWORKS_INCLUDE_DIR} )

MACRO( FIND_OSGWORKS_LIBRARY MYLIBRARY MYLIBRARYNAME )
    MARK_AS_ADVANCED( ${MYLIBRARY} )
    MARK_AS_ADVANCED( ${MYLIBRARY}_debug )
    FIND_LIBRARY( ${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
        PATHS
        $ENV{OSGWORKS_BUILD_DIR}/lib
        $ENV{OSGWORKS_BUILD_DIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        "C:/Program Files/OSGWorks/lib"
        "C:/Program Files (x86)/OSGWorks/lib"
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )
    FIND_LIBRARY( ${MYLIBRARY}_debug
        NAMES ${MYLIBRARYNAME}d
        PATHS
        $ENV{OSGWORKS_BUILD_DIR}/lib
        $ENV{OSGWORKS_BUILD_DIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        "C:/Program Files/OSGWorks/lib"
        "C:/Program Files (x86)/OSGWorks/lib"
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )
#    message( STATUS ${${MYLIBRARY}} ${${MYLIBRARY}_debug} )
#    message( STATUS ${MYLIBRARYNAME} )
    IF( ${MYLIBRARY}_debug )
        SET( OSGWORKS_LIBRARIES ${OSGWORKS_LIBRARIES}
            "optimized" ${${MYLIBRARY}}
            "debug" ${${MYLIBRARY}_debug}
        )
    ELSE( ${MYLIBRARY}_debug )
        SET( OSGWORKS_LIBRARIES ${OSGWORKS_LIBRARIES} ${${MYLIBRARY}} )
    ENDIF( ${MYLIBRARY}_debug )
ENDMACRO(FIND_OSGWORKS_LIBRARY LIBRARY LIBRARYNAME)

FIND_OSGWORKS_LIBRARY( OSGTOOLS_LIBRARY osgTools )
FIND_OSGWORKS_LIBRARY( OSGCONTROLS_LIBRARY osgControls )

IF( OSGWORKS_LIBRARIES AND OSGWORKS_INCLUDE_DIR )
    SET( OSGWORKS_FOUND "YES")
ELSE( OSGWORKS_LIBRARIES AND OSGWORKS_INCLUDE_DIR )
    SET( OSGWORKS_FOUND "NO" )
ENDIF( OSGWORKS_LIBRARIES AND OSGWORKS_INCLUDE_DIR )
