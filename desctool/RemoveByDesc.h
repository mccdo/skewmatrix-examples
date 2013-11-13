#ifndef __REMOVE_BY_DESC_H__
#define __REMOVE_BY_DESC_H__ 1


#include <osg/NodeVisitor>

#include <set>
#include <string>



/** RemoveByDesc RemoveByDesc.h
\brief Remove nodes with a specified description string.
\details TBD
**/
class RemoveByDesc : public osg::NodeVisitor
{
public:
    RemoveByDesc();

    int execute( osg::Node* node=NULL );

    void setDescriptions( const osg::Node::DescriptionList& desc );
    const osg::Node::DescriptionList& getDescriptions() const;

    virtual void apply( osg::Node& node );
    virtual void apply( osg::MatrixTransform& node );
    virtual void apply( osg::Geode& node );

protected:
    osg::Node::DescriptionList _desc;

    typedef std::set< osg::ref_ptr< osg::Node > > NodeSet;
    NodeSet _nodes;
};


// __REMOVE_BY_DESC_H__
#endif
