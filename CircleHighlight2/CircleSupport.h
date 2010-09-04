// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __CIRCLE_SUPPORT_H__
#define __CIRCLE_SUPPORT_H__ 1

#include <osg/Node>
#include <osg/Drawable>
#include <osg/StateSet>
#include <osg/Program>
#include <osg/Vec3>
#include <osg/ref_ptr>

#include <string>


class CircleSupport
{
public:
    CircleSupport();

    osg::Node* createCircleHighlight( const osg::NodePath& nodePath, const osg::Node& pickedNode );
    void createCircleState( osg::StateSet* ss );
    osg::Drawable* createPoint();

    void setLabelText( const std::string& labelText );
    const std::string& getLabelText() const;

protected:
    virtual ~CircleSupport();

    std::string _labelText;

    osg::ref_ptr< osg::Program > _lineStripProgram;
    osg::ref_ptr< osg::Program > _textProgram;
};


//  __CIRCLE_SUPPORT_H__
#endif
