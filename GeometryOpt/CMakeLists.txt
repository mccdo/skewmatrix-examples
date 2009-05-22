ADD_EXECUTABLE( GeometryOpt
    GeometryOpt.cpp
    OptVisitor.cpp
    OptVisitor.h
    CountsVisitor.cpp
    CountsVisitor.h
)

LINK_INTERNAL(
    GeometryOpt
    osgViewer
    osgGA
    osgText
    osgDB
    osgUtil
    osg
    OpenThreads
)
