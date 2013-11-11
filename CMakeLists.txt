cmake_minimum_required(VERSION 2.6)

# # # # # #  project setup  # # # # # #
PROJECT( ISUExamples )
SET( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )
SET( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all libraries." )
MARK_AS_ADVANCED( LIBRARY_OUTPUT_PATH )
SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all executables." )
SET(CMAKE_DEBUG_POSTFIX  "d")
MARK_AS_ADVANCED( EXECUTABLE_OUTPUT_PATH )
MAKE_DIRECTORY( ${PROJECT_BINARY_DIR}/bin )

# Define macro that helps us append 'd' to debug executables/libs on Windows
INCLUDE( CMakeMacros )

include( FindOSGHelper )
include( FindBulletHelper )
include_directories( ${BULLET_INCLUDE_DIR} )

FIND_PACKAGE( osgWorks 2.0.0 REQUIRED )
if( osgWorks_FOUND )
    include( ${osgWorks_USE_FILE} )
endif()

find_package( OpenGL REQUIRED )
FIND_PACKAGE( osgBullet 2.0.0 REQUIRED )
if( osgBullet_FOUND )
    include( ${osgBullet_USE_FILE} )
endif()
find_package( OpenAL )
find_package( DirectInput QUIET )

find_package(osgAudio REQUIRED)
if( osgAudio_FOUND )
   include(${osgAudio_USE_FILE})
endif()

# OpenAL/OpenAL++ audio subsystem support
IF(ENABLE_SUBSYSTEM_OPENAL)
    SET(ALUT_USE_AL_SUBDIR ON)
    FIND_PACKAGE(ALUT)
    if(ALUT_FOUND)
        include_directories(${ALUT_INCLUDE_DIR})
    endif(ALUT_FOUND)
    FIND_PACKAGE(OpenAL)
    if(OPENAL_FOUND)
        include_directories(${OPENAL_INCLUDE_DIR})
    endif(OPENAL_FOUND)
    FIND_PACKAGE(OGG)
    if(OGG_FOUND)
        include_directories(${OGG_INCLUDE_DIR})
    endif(OGG_FOUND)
    FIND_PACKAGE(Vorbis)
    if(VORBIS_FOUND)
        include_directories(${VORBIS_INCLUDE_DIR})
    endif(VORBIS_FOUND)
    FIND_PACKAGE(VorbisFile)
ENDIF(ENABLE_SUBSYSTEM_OPENAL)

# FMOD Ex support
IF(ENABLE_SUBSYSTEM_FMOD)
    FIND_PACKAGE(FMOD)
    if( FMOD_FOUND )
        include_directories(${FMOD_INCLUDE_DIR})
    endif(FMOD_FOUND)
ENDIF(ENABLE_SUBSYSTEM_FMOD)

set(BOOST_INCLUDEDIR "$ENV{BOOST_INSTALL_DIR}/include" CACHE PATH "docstring")

# Must use Boost as dynamic libs.
add_definitions( -DBOOST_ALL_DYN_LINK )
set(Boost_USE_STATIC_LIBS       OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

set( BOOST_ROOT "Please specify" CACHE PATH "Root directory where Boost is installed" )
find_package(Boost 1.44.0 REQUIRED COMPONENTS regex)

GET_FILENAME_COMPONENT( OSG_LIBRARIES_DIR ${OSG_LIBRARY} PATH)
INCLUDE_DIRECTORIES(
    ${OSG_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)
LINK_DIRECTORIES(
    ${OSG_LIBRARIES_DIR}
)

# benchmarks
ADD_SUBDIRECTORY( uniformperf )

# Support for Tony's Maya models
ADD_SUBDIRECTORY( mayaviewer )
ADD_SUBDIRECTORY( texmerge )

# examples
ADD_SUBDIRECTORY( AutoTransform2 )
ADD_SUBDIRECTORY( character )
ADD_SUBDIRECTORY( checkindices )
ADD_SUBDIRECTORY( CircleHighlight )
ADD_SUBDIRECTORY( CircleHighlight2 )
ADD_SUBDIRECTORY( ComplexSurfaces )
ADD_SUBDIRECTORY( DepthOfField )
ADD_SUBDIRECTORY( DepthOfField2 )
ADD_SUBDIRECTORY( depthpartition )
ADD_SUBDIRECTORY( DepthPeel )
ADD_SUBDIRECTORY( desctool )
ADD_SUBDIRECTORY( dwgvisitor )
ADD_SUBDIRECTORY( GeometryOpt )
ADD_SUBDIRECTORY( gl3SafeCow )
ADD_SUBDIRECTORY( glow )
ADD_SUBDIRECTORY( indexcheck )
ADD_SUBDIRECTORY( lighting )
ADD_SUBDIRECTORY( meshopt )
add_subdirectory( mipmaplimit )
ADD_SUBDIRECTORY( mrtclear )
ADD_SUBDIRECTORY( opacityvisitor )
ADD_SUBDIRECTORY( nodeshare )
ADD_SUBDIRECTORY( oqdebug )
ADD_SUBDIRECTORY( rttcapture )
ADD_SUBDIRECTORY( shadertext )
ADD_SUBDIRECTORY( Slider )
ADD_SUBDIRECTORY( streamlines1 )
ADD_SUBDIRECTORY( streamlines2 )
ADD_SUBDIRECTORY( streamlines3 )
ADD_SUBDIRECTORY( tankvis )
ADD_SUBDIRECTORY( textureatlas )
ADD_SUBDIRECTORY( TextureSubload )
ADD_SUBDIRECTORY( uniqifierstate )
ADD_SUBDIRECTORY( VectorField )


# Bullet examples
# message( STATUS "Bullet: " ${BULLET_FOUND} )
# message( STATUS "osgBullet " ${OSGBULLET_FOUND} )

if( BULLET_FOUND AND osgBullet_FOUND )

#   OSGBULLET_USE_DOUBLE_PRECISION gets set to TRUE or FALSE in FindBulletHelper.cmake.
#   If TRUE, add the BT double precision definition globally.
    if( OSGBULLET_USE_DOUBLE_PRECISION )
        add_definitions( -DBT_USE_DOUBLE_PRECISION )
    endif()

    # TBD add bullet-specific examples here
    #ADD_SUBDIRECTORY( bullet_tbd )
    add_subdirectory( ghostchar )
    INCLUDE_DIRECTORIES(
    ${BULLET_INCLUDE_DIR} )
#   Collision-triggered sound examples
    if( osgAudio_FOUND )
        ADD_SUBDIRECTORY( bulletsound )
    endif()
endif()

if( OSGAUDIO_FOUND )
#    ADD_SUBDIRECTORY( SoundStateCull )
endif()
