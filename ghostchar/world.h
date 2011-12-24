// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#ifndef __WORLD_H__
#define __WORLD_H__ 1


#include <osg/node>
#include <osg/NodeVisitor>
#include <btBulletDynamicsCommon.h>


osg::Node* build();

class EnablePhysicsVisitor : public osg::NodeVisitor
{
public:
    EnablePhysicsVisitor( btDynamicsWorld* bw );
    ~EnablePhysicsVisitor();

    void apply( osg::Transform& node );

protected:
    btDynamicsWorld* _bw;
};


// __WORLD_H__
#endif
