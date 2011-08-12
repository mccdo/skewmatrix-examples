// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osg/io_utils>

#include "TankData.h"



class KeyHandler: public osgGA::GUIEventHandler
{
public:
    KeyHandler( TankDataVector tdv ) : _tdv( tdv ) {}

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
    {
        if( ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN )
        {
            unsigned int idx;
            switch( ea.getKey() )
            {
            case osgGA::GUIEventAdapter::KEY_Up:
            {
                for( idx=0; idx<_tdv.size(); idx++ )
                {
                    TankData* td = _tdv[ idx ];
                    float pct = td->getPercentOfCapacity();
                    td->setPercentOfCapacity( pct + .01 );
                }
                return( true );
                break;
            }
            case osgGA::GUIEventAdapter::KEY_Down:
            {
                for( idx=0; idx<_tdv.size(); idx++ )
                {
                    TankData* td = _tdv[ idx ];
                    float pct = td->getPercentOfCapacity();
                    td->setPercentOfCapacity( pct - .01 );
                }
                return( true );
                break;
            }
            }
        }
        return( false );
    }

protected:
    TankDataVector _tdv;
};

int main( int argc, char** argv )
{
    osg::notify( osg::ALWAYS ) << "Usage: tankvis <file> [options]" << std::endl;
    osg::notify( osg::ALWAYS ) << "Options:" << std::endl;
    osg::notify( osg::ALWAYS ) << "    -u <x> <y> <z>\tUp vector axis. Default: '-up 0 0 1'." << std::endl;
    osg::notify( osg::ALWAYS ) << "    -c <r> <g> <b> <a>\tFluid color. Default: '-c 0 .8 .8 1'." << std::endl;
    osg::notify( osg::ALWAYS ) << "\nUse the UP and DOWN arrow keys to change percent of capacity at runtime." << std::endl;
    osg::notify( osg::ALWAYS ) << std::endl;

    if( argc == 1 )
    {
        osg::notify( osg::FATAL ) << "Please specify a model file name." << std::endl;
        return( 1 );
    }
    osg::ArgumentParser arguments( &argc, argv );

    osg::notify( osg::ALWAYS ) << "Using:" << std::endl;
    osg::Vec3 up( 0., 0., 1. );
    float x, y, z;
    if( arguments.read( "-up", x, y, z ) )
        up.set( x, y, z );
    osg::notify( osg::NOTICE ) << "    Up: " << up << std::endl;

    osg::Vec4 color( 0., .8, .8, 1. );
    float r, g, b, a;
    if( arguments.read( "-c", r, g, b, a ) )
        color.set( r, g, b, a );
    osg::notify( osg::NOTICE ) << "    Color: " << color << std::endl;

    TankDataVector tdv;
    osg::ref_ptr< osg::Group > grp = new osg::Group;
    while( arguments.argc() > 1 )
    {
        osg::Node* model = osgDB::readNodeFile( arguments[ 1 ] );
        arguments.remove( 1 );
        grp->addChild( model );

        TankData* td = new TankData( model );
        td->setUp( up );
        td->setColorMethod( TankData::COLOR_EXPLICIT );
        td->setExplicitColor( color );
        td->setPercentOfCapacity( 0.5f );

        tdv.push_back( td );
    }
    osg::notify( osg::NOTICE ) << "    Found: " << grp->getNumChildren() <<
        " model" << ( ( grp->getNumChildren()>1 ) ? "s" : "" ) <<
        " on the command line." << std::endl;

    osgViewer::Viewer viewer;
    viewer.setSceneData( grp.get() );
    osg::ref_ptr< KeyHandler > kh = new KeyHandler( tdv );
    viewer.addEventHandler( kh.get() );

    viewer.run();

    unsigned int idx;
    for( idx=0; idx<tdv.size(); idx++ )
        delete tdv[ idx ];
}
