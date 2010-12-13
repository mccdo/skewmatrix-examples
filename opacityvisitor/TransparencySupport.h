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
\return True if the node has a StateSet and the StateSet has the following signature:
\li A BlendColor StateAttribute
\li A BlendFunc StateAttribute
\li GL_BLEND is enabled
\li Rendering hint set to TRANSPARENT_BIN
*/
bool isTransparent( const osg::Node* node );




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
bool isTransparent( const osg::StateSet* stateSet );




/** \brief Find StateSets with nominal transparency, and mark the transparent state as PROTECTED.
Run this node visitor on scene graphs / loaded models that potentially already
contain transparency. The visitor marks the transparent state as PROTECTED so that
subsequently enabling transparency on an ancestor node will not affect the
protected state.
*/
class ProtectTransparencyVisitor : public osg::NodeVisitor
{
public:
    ProtectTransparencyVisitor();

    virtual void apply( osg::Node& node );
    virtual void apply( osg::Geode& geode );

protected:
    /** \bried Mark the transparent componenets of \c stateSet as PROTECTED.
    Does nothing if the \c stateSet is NULL.
    */
    virtual void protectTransparent( osg::StateSet* stateSet ) const;

    /** \brief A general test for transparency.
    Code was lifted from osgconv.cpp FixTransparentVisitor and modified.
    */
    virtual bool isTransparentInternal( const osg::StateSet* stateSet ) const;
};




/** \brief Recursively restore opacity on a subgraph.
This visitor should be considered part of the implementation of transparentDisable(),
which invokes this visitor when the \c recursive parameter is true.
Therefore, apps should call transparentDisable(), rather than invoking this
visitor directly.
*/
class RestoreOpacityVisitor : public osg::NodeVisitor
{
public:
    RestoreOpacityVisitor();

    virtual void apply( osg::Node& node );
    virtual void apply( osg::Geode& geode );
};


// __TRANSPARENCY_SUPPORT_H__
#endif
