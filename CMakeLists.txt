cmake_minimum_required(VERSION 2.4)

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


FIND_PACKAGE( osg REQUIRED )

# Required if any tests make direct calls into OpenGL.
# FIND_LIBRARY( gl REQUIRED )

GET_FILENAME_COMPONENT( OSG_LIBRARIES_DIR ${OSG_LIBRARY} PATH)
INCLUDE_DIRECTORIES(
    ${OSG_INCLUDE_DIR}
)
LINK_DIRECTORIES(
    ${OSG_LIBRARIES_DIR}
)


ADD_SUBDIRECTORY( DIStreamlines )
ADD_SUBDIRECTORY( GeometryOpt )
ADD_SUBDIRECTORY( WarpVertex )
