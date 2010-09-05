// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __CIRCLE_SUPPORT_H__
#define __CIRCLE_SUPPORT_H__ 1

#include <osg/Node>
#include <osg/NodeCallback>
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

    void setLabelText( const std::string& labelText );
    const std::string& getLabelText() const;

    // Default size: 15 pixels.
    void setLabelSize( float sizePicels );
    float getLabelSize() const;

protected:
    virtual ~CircleSupport();

    void createCircleState( osg::StateSet* ss );
    osg::Drawable* createPoint();

    std::string _labelText;
    float _labelSize;

    osg::ref_ptr< osg::Program > _lineStripProgram;
    osg::ref_ptr< osg::Program > _textProgram;
};


class CameraUpdateViewportCallback : public osg::NodeCallback
{
public:
    CameraUpdateViewportCallback();

    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

protected:
    ~CameraUpdateViewportCallback();
};


//  __CIRCLE_SUPPORT_H__
#endif
