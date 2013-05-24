#ifndef __COMPRESS_SUBGRAPH_VISITOR_H__
#define __COMPRESS_SUBGRAPH_VISITOR_H__ 1


#include <osg/NodeVisitor>

#include <set>
#include <vector>


class CompressSubgraphVisitor : public osg::NodeVisitor
{
public:
    CompressSubgraphVisitor( osg::Node* node=NULL, const unsigned int threshold=5000 );

    /** \brief Group node child count threshold.
    Ignore (don't compredd) subgraphs that are rooted at Group nodes
    with less than \c threshold children. The default is 5000. */
    void setNumChildrenThreshold( const unsigned int threshold );
    unsigned int getNumChildrenThreshold() const;

    virtual void apply( osg::Node& node );
    virtual void apply( osg::Geode& node );
    virtual void apply( osg::Group& node );

protected:
    typedef std::set< osg::StateSet* > StateSetSet;
    typedef std::vector< StateSetSet > StateSetStack;

    StateSetStack _stack;
    unsigned int _threshold;
};


// __COMPRESS_SUBGRAPH_VISITOR_H__
#endif
