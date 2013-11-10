#include "RemoveByDesc.h"
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osgwTools/InsertRemove.h>
#include <osg/ref_ptr>

#include <iostream>


RemoveByDesc::RemoveByDesc()
    : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
}

void RemoveByDesc::setDescriptions( const osg::Node::DescriptionList& desc )
{
    _desc = desc;
}
const osg::Node::DescriptionList& RemoveByDesc::getDescriptions() const
{
    return( _desc );
}


void RemoveByDesc::apply( osg::Node& node )
{
    const osg::Node::DescriptionList& localDesc( node.getDescriptions() );
    if( _desc.size() == localDesc.size() )
    {
        bool mismatch( false );
        for( unsigned int idx=0; idx<_desc.size(); ++idx )
        {
            if( _desc[ idx ] != localDesc[ idx ] )
            {
                mismatch = true;
                break;
            }
        }
        if( !mismatch )
            _nodes.insert( &node );
    }

    traverse( node );
}
void RemoveByDesc::apply( osg::MatrixTransform& node )
{
    traverse( node );
}
void RemoveByDesc::apply( osg::Geode& node )
{
    traverse( node );
}


int RemoveByDesc::execute( osg::Node* node )
{
    if( node != NULL )
        node->accept( *this );

    std::cout << "Found " << _nodes.size() << " nodes." << std::endl;
    if( _nodes.empty() )
        return( 0 );

    unsigned int removed( 0 );
    for( NodeSet::const_iterator it = _nodes.begin(); it != _nodes.end(); ++it )
    {
        osg::ref_ptr< osg::Node > target( *it );
        if( target->getNumParents() > 0 )
        {
            osgwTools::removeNode( target.get() );
            ++removed;
        }
    }

    return( removed );
}
