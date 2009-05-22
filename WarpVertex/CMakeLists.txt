ADD_EXECUTABLE( WarpVertex WarpVertex.cpp )

LINK_INTERNAL(
    WarpVertex
    osgViewer
    osgGA
    osgText
    osgDB
    osgUtil
    osg
    OpenThreads
)
