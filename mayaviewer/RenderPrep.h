// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#ifndef __RENDER_PREP_H__
#define __RENDER_PREP_H__ 1


#include <osgwTools/StateTrackingNodeVisitor.h>


class RenderPrep : public osgwTools::StateTrackingNodeVisitor
{
public:
    RenderPrep( osg::Node* root );
    ~RenderPrep();

    virtual void apply( osg::Node& node );
    virtual void apply( osg::Geode& node );

protected:
    osg::ref_ptr< osg::Uniform > _isOsgTextUniform;
    osg::ref_ptr< osg::Uniform > _noTextureUniform;
    osg::ref_ptr< osg::Uniform > _shadowOnlyUniform;

    void applyDrawable( osg::Drawable* draw );

    void processStateSet( osg::Node& node );
    void processStateSet( osg::Drawable* draw );

    /** Returns true if texture units 0, 1, 2, and 3 are all NULL. */
    bool detectNoTexture( const osg::StateSet* stateSet ) const;

    /** Return true if texture unit 1 is not NULL, and units 0, 2, and 3
    are NULL. */
    bool detectShadowOnly( const osg::StateSet* stateSet ) const;
};


// __RENDER_PREP_H__
#endif
