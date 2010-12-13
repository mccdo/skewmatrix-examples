// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __TRANSPARENCY_SUPPORT_H__
#define __TRANSPARENCY_SUPPORT_H__ 1


#include <osg/NodeVisitor>


bool transparentEnable( osg::Node* node, float alpha );

bool transparentDisable( osg::Node* node, bool recursive=false );

bool isTransparent( osg::Node* node );


class ProtectTransparencyVisitor : public osg::NodeVisitor
{
public:
    ProtectTransparencyVisitor();
    ~ProtectTransparencyVisitor();

    void apply( osg::Node& node );
    void apply( osg::Geode& geode );
};

class RestoreOpacityVisitor : public osg::NodeVisitor
{
public:
    RestoreOpacityVisitor();
    ~RestoreOpacityVisitor();

    void apply( osg::Node& node );
    void apply( osg::Geode& geode );
};


// __TRANSPARENCY_SUPPORT_H__
#endif
