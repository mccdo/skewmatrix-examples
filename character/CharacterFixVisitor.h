// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __CHARACTER_FIX_VISITOR_H__
#define __CHARACTER_FIX_VISITOR_H__ 1

#include <osg/NodeVisitor>

namespace osg { class Geometry; }

class CharacterFixVisitor : public osg::NodeVisitor
{
public:
    CharacterFixVisitor( osg::NodeVisitor::TraversalMode mode=osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );

    // Do not call node->accept(visitor).
    // Instead call this: visitor.process(node) and use the returned
    // value as the top of the model subgraph.
    osg::Node* process( osg::Node& node );

    // Default: 0.032808 (converts centimaters to feet).
    void setScaleFactor( double scale );
    double getScaleFactor() const;

    // Default: true. Normals will be reversed.
    void setReverseNormals( bool reverse );
    bool getReverseNormals() const;

    // Default: strip true, prefix "" (texture paths are stripped, with no prefix).
    // If strip is true, and prefix is "Images/", then the output
    // will reference the texture file "Images/texture.ext".
    // Prefix is ignored if strip is false.
    void setTexturePathControl( bool strip, const std::string& prefix=std::string( "" ) );

    virtual void apply( osg::Group& node );
    virtual void apply( osg::Node& node );
    virtual void apply( osg::Geode& node );

protected:
    void preTraverse( osg::Group& grp );
    void postTraverse( osg::Group& grp );

    void applyStateSet( osg::StateSet* ss );
    void applyGeometry( osg::Geometry* geom );

    double _scale;
    bool _reverseNormals;

    bool _stripTexturePaths;
    std::string _texturePrefix;
};

// __CHARACTER_FIX_VISITOR_H__
#endif
