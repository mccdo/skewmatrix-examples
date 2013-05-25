#ifndef __COMPRESS_SUBGRAPH_VISITOR_H__
#define __COMPRESS_SUBGRAPH_VISITOR_H__ 1


#include <osg/NodeVisitor>
#include <osgUtil/Optimizer>

#include <set>
#include <vector>
#include <string>



/** CompressSubgraphVisitor CompressSubgraphVisitor.h
\brief Compress certain subgraphs within a scene graph.
\details This visitor has two compression modes: Child
Threshold, and Single Name.

In Child Threshold mode, a subgraph is compressed if the
number of children at the root node exceeds the theshold
set with setNumChildrenThreshold.

In Single Name mode, a subgraph is compressed if the number
of unique names in the subgraph (discounting Drawable names)
equals one.

In both modes, a subgraph is compressed only if there is
zero or one unique StateSets referenced in the subgraph.

"Compressed" means the osgUtil::Optimizer is ran on the
scene graph to remove redundant nodes, merge geodes, and
merge geometry.

If you pass a node to the constructor, this visitor performs
two traversals, first in Child Threshold mode, then in Single
Name mode. This is often faster than executing both modes
simultaneously, as the first traversal can dramatically reduce
the processing of the second traversal for certain scene graphs.
**/
class CompressSubgraphVisitor : public osg::NodeVisitor
{
public:
    CompressSubgraphVisitor( osg::Node* node=NULL, const unsigned int threshold=5000 );

    /** \brief Group node child count threshold.
    Ignore (don't compredd) subgraphs that are rooted at Group nodes
    with less than \c threshold children. The default is 5000. */
    void setNumChildrenThreshold( const unsigned int threshold );
    unsigned int getNumChildrenThreshold() const;

    enum {
        CHILD_THRESHOLD = ( 0x1 << 0 ),
        SINGLE_NAME = ( 0x1 << 1 ),
        FULL = ( CHILD_THRESHOLD | SINGLE_NAME )
    };
    void setCompressionMode( const unsigned int flags );
    unsigned int getCompressionMode() const;

    virtual void apply( osg::Node& node );
    virtual void apply( osg::Geode& node );
    virtual void apply( osg::Group& node );

    struct LocalIsOpPermissible : public osgUtil::Optimizer::IsOperationPermissibleForObjectCallback
    {
        virtual bool isOperationPermissibleForObjectImplementation( const osgUtil::Optimizer* optimizer, const osg::Node* node, unsigned int option ) const;
    };

protected:
    osgUtil::Optimizer _opt;

    bool _thresholdCheck;
    bool _nameCheck;

    unsigned int _threshold;

    typedef std::set< osg::StateSet* > StateSetSet;
    typedef std::vector< StateSetSet > StateSetStack;

    StateSetStack _stateStack;

    typedef std::set< std::string > NameSet;
    typedef std::vector< NameSet > NameStack;

    NameStack _nameStack;
};


// __COMPRESS_SUBGRAPH_VISITOR_H__
#endif
