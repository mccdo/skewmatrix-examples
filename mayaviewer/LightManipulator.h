// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#ifndef __LIGHT_MANIPULATOR_H__
#define __LIGHT_MANIPULATOR_H__ 1


#include <osgGA/GUIEventHandler>

class LightManipulator : public osgGA::GUIEventHandler
{
public:
    LightManipulator();

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

    osg::Node* getLightSubgraph();

protected:
    virtual ~LightManipulator();

    osg::ref_ptr< osg::Node > _lightSubgraph;
    float _scale;
    osg::Vec3 _position;
};


// __LIGHT_MANIPULATOR_H__
#endif
