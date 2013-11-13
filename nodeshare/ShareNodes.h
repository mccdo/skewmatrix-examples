#ifndef __SHARE_NODES_H__
#define __SHARE_NODES_H__ 1


#include <osg/NodeVisitor>

#include <map>
#include <set>
#include <string>



/** ShareNodes ShareNodes.h
\brief Share nodes with indentical names.
\details Traverse the scene graph to build a map of node names
to std::list of nodes with those names. During the execute() call,
keep only once instance of each named subgraph and share it as
appropriate. **/
class ShareNodes : public osg::NodeVisitor
{
public:
    ShareNodes();

    void execute( osg::Node* node=NULL );

    virtual void apply( osg::Node& node );
    /* MatrixTransforms will not be considered for sharing. */
    virtual void apply( osg::MatrixTransform& node );
    /* Geodes will not be considered for sharing. */
    virtual void apply( osg::Geode& node );

protected:
    typedef std::set< osg::ref_ptr< osg::Node > > NodeSet;
    typedef std::map< std::string, NodeSet > NameMap;
    NameMap _names;
};


// __SHARE_NODES_H__
#endif
