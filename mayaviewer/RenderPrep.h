// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#ifndef __RENDER_PREP_H__
#define __RENDER_PREP_H__ 1


#include <osgwTools/StateTrackingNodeVisitor.h>


class RenderPrep : public osgwTools::StateTrackingNodeVisitor
{
public:
    RenderPrep( osg::Node* root, const float textSize=100.f, bool noPM=false );
    ~RenderPrep();

    virtual void apply( osg::Node& node );
    virtual void apply( osg::Geode& node );

protected:
    float _textSize;

    osg::ref_ptr< osg::Uniform > _isOsgTextUniform;

    void applyDrawable( osg::Drawable* draw );

    void processStateSet( osg::Node& node );
    void processStateSet( osg::Drawable* draw );
};


// __RENDER_PREP_H__
#endif
