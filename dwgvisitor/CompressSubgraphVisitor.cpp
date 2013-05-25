#include "CompressSubgraphVisitor.h"
#include <osg/Geode>
#include <osg/Group>
#include <osg/Drawable>
#include <osg/ref_ptr>
#include <osgUtil/Optimizer>


CompressSubgraphVisitor::CompressSubgraphVisitor( osg::Node* node, const unsigned int threshold )
    : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
    _thresholdCheck( true ),
    _nameCheck( false ),
    _threshold( threshold )
{
    _opt.setIsOperationPermissibleForObjectCallback( new LocalIsOpPermissible() );

    if( node != NULL )
    {
        OSG_ALWAYS << "Child threshold pass..." << std::endl;
        node->accept( *this );

        OSG_ALWAYS << "Single name pass..." << std::endl;
        setCompressionMode( SINGLE_NAME );
        node->accept( *this );
    }
}

void CompressSubgraphVisitor::setNumChildrenThreshold( const unsigned int threshold )
{
    _threshold = threshold;
}
unsigned int CompressSubgraphVisitor::getNumChildrenThreshold() const
{
    return( _threshold );
}

void CompressSubgraphVisitor::setCompressionMode( const unsigned int flags )
{
    _thresholdCheck =  _nameCheck = false;
    if( flags & SINGLE_NAME )
        _nameCheck = true;
    if( flags & CHILD_THRESHOLD )
        _thresholdCheck = true;
}
unsigned int CompressSubgraphVisitor::getCompressionMode() const
{
    unsigned int flags( 0 );
    if( _nameCheck )
        flags |= SINGLE_NAME;
    if( _thresholdCheck )
        flags |= CHILD_THRESHOLD;
    return( flags );
}


void CompressSubgraphVisitor::apply( osg::Node& node )
{
    if( ( node.getStateSet() != NULL ) && ( _stateStack.size() > 0 ) )
        _stateStack.back().insert( node.getStateSet() );

    if( _nameCheck &&
        !( node.getName().empty() ) && ( _nameStack.size() > 0 ) )
        _nameStack.back().insert( node.getName() );

    traverse( node );
}
void CompressSubgraphVisitor::apply( osg::Geode& node )
{
    if( ( node.getStateSet() != NULL ) && ( _stateStack.size() > 0 ) )
        _stateStack.back().insert( node.getStateSet() );

    if( _nameCheck &&
        !( node.getName().empty() ) && ( _nameStack.size() > 0 ) )
        _nameStack.back().insert( node.getName() );

    if( _stateStack.empty() )
        return;

    for( unsigned int idx = 0; idx < node.getNumDrawables(); ++idx )
    {
        osg::Drawable* draw( node.getDrawable( idx ) );

        if( draw->getStateSet() != NULL )
            _stateStack.back().insert( draw->getStateSet() );

        // Do not check Drawables names. PT never exports Drawables with names.
    }

    traverse( node );
}
void CompressSubgraphVisitor::apply( osg::Group& node )
{
    // Push data onto state stack.
    StateSetSet newSSS;
    if( node.getStateSet() != NULL )
        newSSS.insert( node.getStateSet() );
    _stateStack.push_back( newSSS );

    // Push data onto name stack.
    if( _nameCheck )
    {
        NameSet newNS;
        if( !( node.getName().empty() ) )
            newNS.insert( node.getName() );
        _nameStack.push_back( newNS );
    }


    traverse( node );


    // Obtain results from state stack.
    const StateSetSet& sss( _stateStack.back() );
    if( _stateStack.size() > 1 )
    {
        // Take all the StateSets in this
        // subgraph and insert them into the new top of stack.
        StateSetSet& newTop( _stateStack[ _stateStack.size() - 2 ] );
        newTop.insert( sss.begin(), sss.end() );
    }
    const unsigned int numStateSetsInSubgraph( sss.size() );
    _stateStack.pop_back();

    // Obtain results from name stack.
    unsigned int numNamesInSubgraph( 0 );
    if( _nameCheck )
    {
        const NameSet& ns( _nameStack.back() );
        if( _nameStack.size() > 1 )
        {
            // Take all the Names in this
            // subgraph and insert them into the new top of stack.
            NameSet& newTop( _nameStack[ _nameStack.size() - 2 ] );
            newTop.insert( ns.begin(), ns.end() );
        }
        numNamesInSubgraph = ns.size();
        _nameStack.pop_back();
    }

    // Check for conditions that force us to skip compression..
    if( numStateSetsInSubgraph > 1 )
    {
        // Too many StateSets
        return;
    }
    if( _nameCheck &&
        ( numNamesInSubgraph > 1 ) )
    {
        // Too many names in the subgraph.
        return;
    }
    if( _thresholdCheck &&
        ( node.getNumChildren() < _threshold ) )
    {
        // Not enough children at this node.
        return;
    }


    // Either the number of named nodes is == 1
    // or the number of children exceeds the threshold.
    // Compress this subgraph.
    OSG_INFO << "Compressing subgraph at node \"" << node.getName() << "\", " <<
        node.getNumChildren() << " children..." << std::endl;

    // Compress the subgraph using the osgUtil::Optimizer.
    //
    // Three-stage optimization:
    const osg::Object::DataVariance savedVariance( node.getDataVariance() );
    node.setDataVariance( osg::Object::DYNAMIC );
    if( node.getNumChildren() > 0 )
    {
        // 1. Detect static objects and remove redundant Groups.
        _opt.reset();
        _opt.optimize( &node,
            osgUtil::Optimizer::STATIC_OBJECT_DETECTION |
            osgUtil::Optimizer::REMOVE_REDUNDANT_NODES );
    }
    if( node.getNumChildren() > 0 )
    {
        // 2. Remove redundant Groups again, just in case.
        _opt.reset();
        _opt.optimize( &node,
            osgUtil::Optimizer::REMOVE_REDUNDANT_NODES );
    }
    if( node.getNumChildren() > 0 )
    {
        // 3. Morge Geodes, Geometry objects, and PrimitiveSets.
        _opt.reset();
        _opt.optimize( &node,
            osgUtil::Optimizer::MERGE_GEODES |
            osgUtil::Optimizer::MERGE_GEOMETRY );
    }

    node.setDataVariance( savedVariance );
}

// osgUtil::Optimizer REMOVE_REDUNDANT_NODES will actually optimize
// away the root node if it thinks it can do so. That's very bad for
// our usage, as we're invoking the Optimizer from a visitor and the
// visitor would end up with an invalid reference. For this reason,
// we add a callback to prevent optimization for certain conditions
// that might result in deleting the root node of the Optimizer traversal.
bool CompressSubgraphVisitor::LocalIsOpPermissible::isOperationPermissibleForObjectImplementation(
        const osgUtil::Optimizer* optimizer, const osg::Node* node, unsigned int option ) const
{
    if( node->getDataVariance() == osg::Object::DYNAMIC )
    {
        // Condition 1: Don't remove a node marked as DYNAMIC.
        return( false );
    }
    else
    {
        const osg::Geode* geode( node->asGeode() );
        if( ( geode != NULL ) &&
            ( geode->getNumDrawables() == 0 ) )
        {
            // Condition 2: Don't remove a Geode with 0 drawables. If the traversal
            // root is a Group with a 0-drawable Geode child, the Optimizer will
            // remove both the Geode and THEN ALSO REMOVE THE GROUP WITHOUT CHECKING
            // THIS CALLBACK (which means the DYNAMIC check alone is insufficient).
            // This seems like a bug in the Optimizer.
            return( false );
        }
        else
            return( optimizer->isOperationPermissibleForObjectImplementation( node, option ) );
    }
}
