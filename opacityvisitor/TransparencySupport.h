// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __TRANSPARENCY_SUPPORT_H__
#define __TRANSPARENCY_SUPPORT_H__ 1


#include <osg/NodeVisitor>


/** \brief Set a transparent StateSet on the given node, saving its current StateSet as UserData.
If UserData is NULL, current StateSet is saved as UserData for later restore.
If there is no current StateSet, one is created.
The current StateSet is modified as follows:
\li BlendColor is set using the specified alpha value.
\li BlendFunc is set to use the BlendColor alpha.
\li The rendering hint is set to TRANSPARENT_BIN.
*/
bool transparentEnable( osg::Node* node, float alpha );

/** \brief Restores opacity by undoing the effects of a prior call to transparentEnable.
If the node isn't transparent (as defined by the isTransparent call),
do nothing and return false. Otherwise, copy the node's UserData
to its StateSet.
\param recursive If true, use the RestoreOpacityVisitor to recursively restore opacity. Default is false.
\return false if \c node is NULL or \c node doesn't have a StateSet. Otherwise, returns true.
*/
bool transparentDisable( osg::Node* node, bool recursive=false );

/** \brief Determine whether a node is transparent.
\return True if the node has a StateSet and the StateSet has rendering hint set to TRANSPARENT_BIN. Otherwise, returns false.
*/
bool isTransparent( osg::Node* node );

/** \brief Enable transparency for the given StateSet using the given alpha value.
This is useful for setting transparency on Drawables instead of Nodes.
*/
bool transparentEnable( osg::StateSet* stateSet, float alpha );

/** \brief Restore opacity by undoing the effects of a prior call to reansparentEnable().
This is useful for restoring opacity on Drawables instead of Nodes.
*/
bool transparentDisable( osg::Drawable* drawable );

/** \brief Return true if the given StateSet has the TRANSPARENT_BIN rendering hint.
This is useful for testing the transparency of Drawables instead of Nodes.
*/
bool isTransparent( osg::StateSet* stateSet );



class ProtectTransparencyVisitor : public osg::NodeVisitor
{
public:
    ProtectTransparencyVisitor();
    ~ProtectTransparencyVisitor();

    void apply( osg::Node& node );
    void apply( osg::Geode& geode );
};

/** \brief recursively restore opacity on a subgraph.
*/
class RestoreOpacityVisitor : public osg::NodeVisitor
{
public:
    RestoreOpacityVisitor();

    void apply( osg::Node& node );
    void apply( osg::Geode& geode );
};


// __TRANSPARENCY_SUPPORT_H__
#endif
