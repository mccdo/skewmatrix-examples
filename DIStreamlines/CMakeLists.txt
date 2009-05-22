ADD_EXECUTABLE( DIStreamlines
    DIStreamlines.cpp
)

LINK_INTERNAL(
    DIStreamlines
    osgViewer
    osgGA
    osgText
    osgDB
    osgUtil
    osg
    OpenThreads
)
