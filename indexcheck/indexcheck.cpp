// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgViewer/Viewer>
#include <osg/NodeVisitor>
#include <osg/Geometry>
#include <osgwTools/Version.h>

#include <string>


// Validates indices in a DrawElements. If index is out of range with 
// respect to the vertex array size, display a warning message. If
// _fix is true (default), set bad indices to the last known good index.
class IndexCheck : public osg::NodeVisitor
{
public:
    IndexCheck()
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
    {
        _geodeCount = _geomCount = 0;
        _fix = true;
    }
    ~IndexCheck() {}

    void apply( osg::Geode& geode )
    {
        unsigned int idx;
        for( idx = 0; idx < geode.getNumDrawables(); idx++ )
        {
            _geodeCount++;

            osg::Geometry* geom =
                dynamic_cast< osg::Geometry* >( geode.getDrawable( idx ) );
            if( geom )
                apply( geom );
        }
        traverse( geode );
    }

    void apply( osg::Geometry* geom )
    {
        _geomCount++;

        const unsigned int vertexSize = geom->getVertexArray()->getNumElements();

        unsigned int idx;
        for( idx=0; idx<geom->getNumPrimitiveSets(); idx++ )
        {
            unsigned int lastGood = 0;
            osg::PrimitiveSet* ps = geom->getPrimitiveSet( idx );
            switch( ps->getType() )
            {
            case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
            {
                osg::DrawElementsUByte* de = static_cast< osg::DrawElementsUByte* >( ps );
                osg::DrawElementsUByte::iterator itr;
                for( itr=de->begin(); itr!=de->end(); itr++ )
                {
                    const unsigned int value = (unsigned int)( *itr );
                    if( value >= vertexSize )
                    {
                        osg::notify( osg::ALWAYS ) << "Index value: " << value << ", vertex array size: " << vertexSize << std::endl;
                        if( _fix )
                            *itr = lastGood;
                    }
                    else
                        lastGood = value;
                }
                break;
            }
            case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
            {
                osg::DrawElementsUShort* de = static_cast< osg::DrawElementsUShort* >( ps );
                osg::DrawElementsUShort::iterator itr;
                for( itr=de->begin(); itr!=de->end(); itr++ )
                {
                    const unsigned int value = (unsigned int)( *itr );
                    if( value >= vertexSize )
                    {
                        osg::notify( osg::ALWAYS ) << "Index value: " << value << ", vertex array size: " << vertexSize << std::endl;
                        if( _fix )
                            *itr = lastGood;
                    }
                    else
                        lastGood = value;
                }
                break;
            }
            case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
            {
                osg::DrawElementsUInt* de = static_cast< osg::DrawElementsUInt* >( ps );
                osg::DrawElementsUInt::iterator itr;
                for( itr=de->begin(); itr!=de->end(); itr++ )
                {
                    const unsigned int value = (unsigned int)( *itr );
                    if( value >= vertexSize )
                    {
                        osg::notify( osg::ALWAYS ) << "Index value: " << value << ", vertex array size: " << vertexSize << std::endl;
                        if( _fix )
                            *itr = lastGood;
                    }
                    else
                        lastGood = value;
                }
                break;
            }
            }
        }
    }

    unsigned int _geodeCount, _geomCount;
    bool _fix;
};


int
main( int argc, char** argv )
{
    if( argc != 2 )
    {
        osg::notify( osg::ALWAYS ) << "Usage:\n\tindexcheck <filename>" << std::endl;
        return( 1 );
    }

    osgDB::FilePathList dfpl = osgDB::getDataFilePathList();
    dfpl.push_back( std::string( "C:/Projects/temp5/data_for_paul" ) );
    dfpl.push_back( std::string( "C:/Projects/temp5/data_for_paul/bad_test_Data" ) );
    osgDB::setDataFilePathList( dfpl );

    std::string fileName( argv[ 1 ] );
    osg::ref_ptr< osg::Node > root = osgDB::readNodeFile( fileName );
    if( !root.valid() )
        return( 1 );
    osg::notify( osg::ALWAYS ) << "Loaded " << fileName << " successfully." << std::endl;

    IndexCheck ic;
    // ic._fix = false; /* Just warn, don't fix bad indices */
    root->accept( ic );
    osg::notify( osg::ALWAYS ) << " Geodes: " <<
        ic._geodeCount << ", geoms: " << ic._geomCount << std::endl;

    std::string path = osgDB::getFilePath( osgDB::findDataFile( fileName ) );
    osgDB::writeNodeFile( *root, path + "/out.ive" );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );

    return( viewer.run() );
}
