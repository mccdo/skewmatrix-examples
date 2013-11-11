#include "ShareNodes.h"
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osgwTools/NodeUtils.h>
#include <osg/ref_ptr>

#include <iostream>


ShareNodes::ShareNodes()
    : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
}


void ShareNodes::apply( osg::Node& node )
{
    if( !( node.getName().empty() ) )
    {
        NodeSet& nodeSet( _names[ node.getName() ] );
        nodeSet.insert( &node );
    }

    traverse( node );
}
void ShareNodes::apply( osg::MatrixTransform& node )
{
    traverse( node );
}
void ShareNodes::apply( osg::Geode& node )
{
    traverse( node );
}


void ShareNodes::execute( osg::Node* node )
{
    if( node != NULL )
        node->accept( *this );

    std::cout << "Found " << _names.size() << " non-NULL names." << std::endl;
    if( _names.empty() )
        return;
    /* Debug
    for( NameMap::const_iterator it = _names.begin(); it != _names.end(); ++it )
    {
        const NodeSet& nodeSet( it->second );
        std::cout << "  \"" << it->first << "\": " << nodeSet.size() << std::endl;
    }
    */

    for( NameMap::const_iterator it = _names.begin(); it != _names.end(); ++it )
    {
        const NodeSet& nodeSet( it->second );
        if( nodeSet.size() < 2 )
            continue;

        NodeSet::const_iterator nodeIt( nodeSet.begin() );
        osg::ref_ptr< osg::Node > primary( *nodeIt );
        ++nodeIt;

        for( ; nodeIt != nodeSet.end(); ++nodeIt )
        {
            osg::ref_ptr< osg::Node > target( *nodeIt );
            osgwTools::replaceSubgraph( primary.get(), target.get() );
        }
    }
}
