#include "CompressSubgraphVisitor.h"
#include <osg/Geode>
#include <osg/Group>
#include <osg/Drawable>
#include <osgUtil/Optimizer>


CompressSubgraphVisitor::CompressSubgraphVisitor( osg::Node* node, const unsigned int threshold )
    : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
    _threshold( threshold )
{
    if( node != NULL )
        node->accept( *this );
}

void CompressSubgraphVisitor::setNumChildrenThreshold( const unsigned int threshold )
{
    _threshold = threshold;
}
unsigned int CompressSubgraphVisitor::getNumChildrenThreshold() const
{
    return( _threshold );
}


void CompressSubgraphVisitor::apply( osg::Node& node )
{
    if( ( node.getStateSet() != NULL ) && ( _stack.size() > 0 ) )
        _stack.back().insert( node.getStateSet() );
    traverse( node );
}
void CompressSubgraphVisitor::apply( osg::Geode& node )
{
    if( _stack.empty() )
        return;

    if( node.getStateSet() != NULL )
        _stack.back().insert( node.getStateSet() );

    for( unsigned int idx = 0; idx < node.getNumDrawables(); ++idx )
    {
        osg::Drawable* draw( node.getDrawable( idx ) );
        if( draw->getStateSet() != NULL )
            _stack.back().insert( draw->getStateSet() );
    }

    traverse( node );
}
void CompressSubgraphVisitor::apply( osg::Group& node )
{
    if( node.getNumChildren() < _threshold )
    {
        // Too few children. Do not attempt to compress this subgraph.
        // But if we have a StateSet, we should register it in the top of
        // the StateSetSet stack.
        if( ( node.getStateSet() != NULL ) && ( _stack.size() > 0 ) )
            _stack.back().insert( node.getStateSet() );
        traverse( node );
        return;
    }

    // More than _threshold children. We will potentially compress this
    // subgraph, only if the entire subgraph uses a single StateSet. Push
    // a new StateSetSet onto top of stack, and insert our StateSet if we
    // have one.
    StateSetSet newSSS;
    if( node.getStateSet() != NULL )
        newSSS.insert( node.getStateSet() );
    _stack.push_back( newSSS );

    traverse( node );

    const StateSetSet& sss( _stack.back() );
    if( _stack.size() > 1 )
    {
        // Take all the StateSets in this
        // subgraph and insert them into the new top of stack.
        StateSetSet& newTop( _stack[ _stack.size() - 2 ] );
        newTop.insert( sss.begin(), sss.end() );
    }

    // Check the result.
    if( sss.size() > 1 )
    {
        // Turns out the subgraph rooted at this node has more than one
        // StateSet, so we can't compress it. 
        OSG_ALWAYS << "Node \"" << node.getName() << "\": unable to compress." << std::endl;
        _stack.pop_back();
        return;
    }


    OSG_ALWAYS << "Node \"" << node.getName() << "\", children: " << node.getNumChildren() << std::endl;

    // Store the StateSet on this node.
    //node.setStateSet( *(sss.begin()) );
    _stack.pop_back();

    // Compress the subgraph using the osgUtil::Optimizer.
    //
    // Note that Optimizer::reset() seems to crash under some circumstances,
    // so I'm just instantiating it three times rather than one time with
    // intervening reset() calls.

    // Three-stage optimization:
    {
        // 1. Detect static objects and remove redundant Groups.
        osgUtil::Optimizer opt;
        opt.optimize( &node,
            osgUtil::Optimizer::STATIC_OBJECT_DETECTION |
            osgUtil::Optimizer::REMOVE_REDUNDANT_NODES );
    }
    {
        // 2. Remove redundant Groups again, just in case.
        osgUtil::Optimizer opt;
        opt.optimize( &node,
            osgUtil::Optimizer::REMOVE_REDUNDANT_NODES );
    }
    {
        // 3. Morge Geodes, Geometry objects, and PrimitiveSets.
        osgUtil::Optimizer opt;
        opt.optimize( &node,
            osgUtil::Optimizer::MERGE_GEODES |
            osgUtil::Optimizer::MERGE_GEOMETRY );
    }

    OSG_ALWAYS << "\tChildren after: " << node.getNumChildren() << std::endl;
}
