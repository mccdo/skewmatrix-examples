/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * osgWorks is (C) Copyright 2009-2011 by Kenneth Mark Bryden
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include <osgwTools/Uniqifier.h>
#include <osgwTools/FindNamedNode.h>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/Node>
#include <osg/Material>
#include <osg/Notify>



void setState( const osg::NodePath& np )
{
    // Save original StateSet as UserData for possible later restore.
    // (Note: Restoring original state is currently not implemented in this example.)
    osg::StateSet* originalState = np.back()->getOrCreateStateSet();
    np.back()->setUserData( originalState );

    // Copy the original state; we'll modify it.
    osg::StateSet* newState = new osg::StateSet( *originalState, osg::CopyOp::DEEP_COPY_ALL );
    np.back()->setStateSet( newState );

    osg::Material* material = new osg::Material;
    material->setEmission( osg::Material::FRONT, osg::Vec4( 1., 0., 0., 1. ) );
    material->setAmbient( osg::Material::FRONT, osg::Vec4( 0., 0., 0., 1. ) );
    material->setDiffuse( osg::Material::FRONT, osg::Vec4( 0., 0., 0., 1. ) );
    material->setSpecular( osg::Material::FRONT, osg::Vec4( 0., 0., 0., 1. ) );

    newState->setAttributeAndModes( material, osg::StateAttribute::ON |
        osg::StateAttribute::OVERRIDE );
}


int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    std::string nodeName;
    if( !( arguments.read( "--nodeName", nodeName ) ) )
    {
        osg::notify( osg::FATAL ) << "Must specify node to highlight by name: \"--nodeName <name>\"." << std::endl;
        return( 1 );
    }

    const bool uniqify( arguments.find( "--uniqify" ) > 0 );

    osg::ref_ptr< osg::Node > root = osgDB::readNodeFiles( arguments );
    if( !( root.valid() ) )
    {
        osg::notify( osg::FATAL ) << "Can't load data file(s)." << std::endl;
        return( 1 );
    }


    osgwTools::FindNamedNode fnn( nodeName );
    fnn.setMatchMethod( osgwTools::FindNamedNode::CONTAINS );
    root->accept( fnn );
    if( fnn._napl.empty() )
    {
        osg::notify( osg::FATAL ) << "Couldn't find node \"" << nodeName << "\"." << std::endl;
        return( 1 );
    }
    osg::notify( osg::ALWAYS ) << "Found " << fnn._napl.size() << " nodes." << std::endl;


    osg::NodePath targetNodePath;
    if( uniqify )
    {
        osg::notify( osg::ALWAYS ) << "Running Uniqifier." << std::endl;
        // Uniqify the first NodePath thatn the FNN found.
        // Return value is a new NodePath containing the new node addresses.
        targetNodePath = osgwTools::uniqify( fnn._napl[ 0 ].second );
    }
    else
        // Not running uniqifier. Target NodePath is the first
        // NodePath that FNN found.
        targetNodePath = fnn._napl[ 0 ].second;


    // Change state on the last node in the target NodePath.
    setState( targetNodePath );


    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 768, 480 );
    viewer.setSceneData( root.get() );

    return( viewer.run() );
}
