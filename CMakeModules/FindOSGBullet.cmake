# Locate OSGWorks
# This module defines:
#   OSGBULLET_FOUND, set to "YES" or "NO".   
#   OSGBULLET_LIBRARIES
#   OSGBULLET_INCLUDE_DIR

MACRO( FIND_OSGBULLET_INCLUDE THIS_OSGBULLET_INCLUDE_DIR THIS_OSGBULLET_INCLUDE_FILE )
    FIND_PATH( ${THIS_OSGBULLET_INCLUDE_DIR} ${THIS_OSGBULLET_INCLUDE_FILE}
        PATHS
            $ENV{OSGBULLET_SOURCE_DIR}
            /usr/local/
            /usr/
            /sw/ # Fink
            /opt/local/ # DarwinPorts
            /opt/csw/ # Blastwave
            /opt/
            "C:/Program Files/OSGBullet"
            "C:/Program Files (x86)/OSGBullet"
            [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/
            ~/Library/Frameworks
            /Library/Frameworks
        PATH_SUFFIXES
            include
    )
ENDMACRO( FIND_OSGBULLET_INCLUDE THIS_OSGBULLET_INCLUDE_DIR THIS_OSGBULLET_INCLUDE_FILE )

FIND_OSGBULLET_INCLUDE( OSGBULLET_INCLUDE_DIR osgBullet/OSGToCollada.h )
# message( STATUS ${OSGBULLET_INCLUDE_DIR} )

MACRO( FIND_OSGBULLET_LIBRARY MYLIBRARY MYLIBRARYNAME )
    MARK_AS_ADVANCED( ${MYLIBRARY} )
    MARK_AS_ADVANCED( ${MYLIBRARY}_debug )
    FIND_LIBRARY( ${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
        PATHS
        $ENV{OSGBULLET_BUILD_DIR}/lib
        $ENV{OSGBULLET_BUILD_DIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        "C:/Program Files/OSGBullet/lib"
        "C:/Program Files (x86)/OSGBullet/lib"
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )
    FIND_LIBRARY( ${MYLIBRARY}_debug
        NAMES ${MYLIBRARYNAME}d
        PATHS
        $ENV{OSGBULLET_BUILD_DIR}/lib
        $ENV{OSGBULLET_BUILD_DIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        "C:/Program Files/OSGBullet/lib"
        "C:/Program Files (x86)/OSGBullet/lib"
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )
#    message( STATUS ${${MYLIBRARY}} ${${MYLIBRARY}_debug} )
#    message( STATUS ${MYLIBRARYNAME} )
    IF( ${MYLIBRARY}_debug )
        SET( OSGBULLET_LIBRARIES ${OSGBULLET_LIBRARIES}
            "optimized" ${${MYLIBRARY}}
            "debug" ${${MYLIBRARY}_debug}
        )
    ELSE( ${MYLIBRARY}_debug )
        SET( OSGBULLET_LIBRARIES ${OSGBULLET_LIBRARIES} ${${MYLIBRARY}} )
    ENDIF( ${MYLIBRARY}_debug )
ENDMACRO(FIND_OSGBULLET_LIBRARY LIBRARY LIBRARYNAME)

FIND_OSGBULLET_LIBRARY( OSGBULLET_LIBRARY osgBullet )

IF( OSGBULLET_LIBRARIES AND OSGBULLET_INCLUDE_DIR )
    SET( OSGULLET_FOUND "YES")
ELSE( OSGBULLET_LIBRARIES AND OSGBULLET_INCLUDE_DIR )
    SET( OSGULLET_FOUND "NO" )
ENDIF( OSGBULLET_LIBRARIES AND OSGBULLET_INCLUDE_DIR )
