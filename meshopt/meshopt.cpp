// Copyright (c) 2013 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgwTools/MeshOptimizers.h>
#include <osgwTools/CountsVisitor.h>
#include <osgUtil/Optimizer>



int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    std::string outFile;
    if( arguments.find( "-o" ) > 0 )
        arguments.read( "-o", outFile );
    if( outFile.empty() )
        outFile = "out.ive";

    osg::ref_ptr< osg::Node > root( osgDB::readNodeFiles( arguments ) );

    OSG_ALWAYS << "Creating tri strips with Optimizer..." << std::endl;
    osgUtil::Optimizer opt;
    opt.optimize( root.get(), osgUtil::Optimizer::TRISTRIP_GEOMETRY );

    osgwTools::CountsVisitor cv0;
    root->accept( cv0 );
    cv0.dump( osg::notify( osg::ALWAYS ) );

    OSG_ALWAYS << "Running TriMeshVisitor..." << std::endl;
    osgUtil::IndexMeshVisitor imv;
    root->accept( imv );
    imv.makeMesh();

    osgwTools::CountsVisitor cv1;
    root->accept( cv1 );

    if( ( cv0.getVertices() != cv1.getVertices() ) ||
        ( cv0.getDrawArrays() != cv1.getDrawArrays() ) )
    {
        OSG_ALWAYS << "TriMeshVisitor results:" << std::endl;
        OSG_ALWAYS << "  Vertices delta: " << (int)cv1.getVertices() - (int)cv0.getVertices() << std::endl;
        OSG_ALWAYS << "  DrawArrays delta: " << (int)cv1.getDrawArrays() - (int)cv0.getDrawArrays() << std::endl;
    }

    OSG_ALWAYS << "Running VertexAccessOrderVisitor..." << std::endl;
    osgUtil::VertexAccessOrderVisitor vaov;
    root->accept( vaov );
    vaov.optimizeOrder();

    OSG_ALWAYS << "Running VertexCacheVisitor..." << std::endl;
    osgUtil::VertexCacheVisitor vcv;
    root->accept( vcv );
    vcv.optimizeVertices();

    osgDB::writeNodeFile( *root, outFile );

    osgwTools::CountsVisitor cv2;
    root->accept( cv2 );
    cv2.dump( osg::notify( osg::ALWAYS ) );

    OSG_ALWAYS << "Running VertexCacheMissVisitor..." << std::endl;
    osgUtil::VertexCacheMissVisitor vcmv;
    root->accept( vcmv );
    OSG_ALWAYS << "VertexCacheMissVisitor results:" << std::endl;
    OSG_ALWAYS << "  Misses: " << vcmv.misses << std::endl;
    OSG_ALWAYS << "  Triangles: " << vcmv.misses << std::endl;

    return( 0 );
}
