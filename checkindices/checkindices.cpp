// Copyright (c) 2013 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osg/NodeVisitor>


class IndexChecker : public osg::NodeVisitor
{
public:
    IndexChecker()
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _error( false )
    {}
    ~IndexChecker() {}

    void apply( osg::Geode& geode );

    bool _error;

protected:
    void validateUByte( osg::PrimitiveSet* ps, const unsigned int minSize );
    void validateUShort( osg::PrimitiveSet* ps, const unsigned int minSize );
    void validateUInt( osg::PrimitiveSet* ps, const unsigned int minSize );
};

void IndexChecker::apply( osg::Geode& geode )
{
    for( unsigned int idx=0; idx< geode.getNumDrawables(); ++idx )
    {
        osg::Geometry* geom( geode.getDrawable( idx )->asGeometry() );
        if( geom == NULL )
            continue;

        unsigned int minSize( geom->getVertexArray()->getNumElements() );
        if( geom->getColorBinding() == osg::Geometry::BIND_PER_VERTEX )
            minSize = osg::minimum< unsigned int >( minSize, geom->getColorArray()->getNumElements() );
        if( geom->getNormalBinding() == osg::Geometry::BIND_PER_VERTEX )
            minSize = osg::minimum< unsigned int >( minSize, geom->getNormalArray()->getNumElements() );
        for( unsigned int unit=0; unit<8; ++unit )
            if( geom->getTexCoordArray( unit ) != NULL )
                minSize = osg::minimum< unsigned int >( minSize, geom->getTexCoordArray( unit )->getNumElements() );

        for( unsigned int pdx=0; pdx<geom->getNumPrimitiveSets(); ++pdx )
        {
            osg::PrimitiveSet* ps( geom->getPrimitiveSet( pdx ) );
            if( ps->getType() == osg::PrimitiveSet::DrawElementsUBytePrimitiveType )
                validateUByte( ps, minSize );
            else if( ps->getType() == osg::PrimitiveSet::DrawElementsUShortPrimitiveType )
                validateUShort( ps, minSize );
            else if( ps->getType() == osg::PrimitiveSet::DrawElementsUIntPrimitiveType )
                validateUInt( ps, minSize );
        }
    }
}

void IndexChecker::validateUByte( osg::PrimitiveSet* ps, const unsigned int minSize )
{
    osg::DrawElementsUByte* de( static_cast< osg::DrawElementsUByte* >( ps ) );
    osg::DrawElementsUByte::iterator iter;
    for( iter=de->begin(); iter!= de->end(); ++iter )
        if( (unsigned int) *iter >= minSize )
        {
            OSG_FATAL << "UByte value " << *iter << " out of range 0 to " << minSize << std::endl;
            _error = true;
        }
}
void IndexChecker::validateUShort( osg::PrimitiveSet* ps, const unsigned int minSize )
{
    osg::DrawElementsUShort* de( static_cast< osg::DrawElementsUShort* >( ps ) );
    osg::DrawElementsUShort::iterator iter;
    for( iter=de->begin(); iter!= de->end(); ++iter )
        if( (unsigned int) *iter >= minSize )
        {
            OSG_FATAL << "UShort value " << *iter << " out of range 0 to " << minSize << std::endl;
            _error = true;
        }
}
void IndexChecker::validateUInt( osg::PrimitiveSet* ps, const unsigned int minSize )
{
    osg::DrawElementsUInt* de( static_cast< osg::DrawElementsUInt* >( ps ) );
    osg::DrawElementsUInt::iterator iter;
    for( iter=de->begin(); iter!= de->end(); ++iter )
        if( (unsigned int) *iter >= minSize )
        {
            OSG_FATAL << "UInt value " << *iter << " out of range 0 to " << minSize << std::endl;
            _error = true;
        }
}



int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::Node* root( osgDB::readNodeFiles( arguments ) );
    IndexChecker ic;
    root->accept( ic );
    if( !ic._error )
        OSG_ALWAYS << "All indices are in range." << std::endl;

    osgViewer::Viewer viewer;
    viewer.setSceneData( root );
    return( viewer.run() );
}
