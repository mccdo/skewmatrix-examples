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

FIND_PACKAGE( OSGWorks REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( OSGBullet )
FIND_PACKAGE( OpenAL )
FIND_PACKAGE( OSGAudio )
find_package( DirectInput QUIET )


GET_FILENAME_COMPONENT( OSG_LIBRARIES_DIR ${OSG_LIBRARY} PATH)
INCLUDE_DIRECTORIES(
    ${OSG_INCLUDE_DIRS}
    ${OSGWORKS_INCLUDE_DIR}
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
ADD_SUBDIRECTORY( GeometryOpt )
ADD_SUBDIRECTORY( gl3SafeCow )
ADD_SUBDIRECTORY( glow )
ADD_SUBDIRECTORY( indexcheck )
ADD_SUBDIRECTORY( lighting )
ADD_SUBDIRECTORY( meshopt )
add_subdirectory( mipmaplimit )
ADD_SUBDIRECTORY( mrtclear )
ADD_SUBDIRECTORY( opacityvisitor )
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
ADD_SUBDIRECTORY( WarpVertex )


# Bullet examples
# message( STATUS "Bullet: " ${BULLET_FOUND} )
# message( STATUS "osgBullet " ${OSGBULLET_FOUND} )

if( BULLET_FOUND AND OSGBULLET_FOUND )

#   OSGBULLET_USE_DOUBLE_PRECISION gets set to TRUE or FALSE in FindBulletHelper.cmake.
#   If TRUE, add the BT double precision definition globally.
    if( OSGBULLET_USE_DOUBLE_PRECISION )
        add_definitions( -DBT_USE_DOUBLE_PRECISION )
    endif()

    INCLUDE_DIRECTORIES(
        ${BULLET_INCLUDE_DIRS}
        ${OSGBULLET_INCLUDE_DIR}
    )
    # TBD add bullet-specific examples here
    #ADD_SUBDIRECTORY( bullet_tbd )
    add_subdirectory( ghostchar )

#   Collision-triggered sound examples
    if( OSGAUDIO_FOUND AND OPENAL_FOUND )
        INCLUDE_DIRECTORIES(
            ${OPENAL_INCLUDE_DIR}
            ${OSGAUDIO_INCLUDE_DIR}
        )
        ADD_SUBDIRECTORY( bulletsound )
    endif()
endif()

if( OSGAUDIO_FOUND )
#    ADD_SUBDIRECTORY( SoundStateCull )
endif()
