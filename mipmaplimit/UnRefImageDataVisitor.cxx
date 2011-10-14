/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2010 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * Date modified: $Date$
 * Version:       $Rev$
 * Author:        $Author$
 * Id:            $Id$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/
#include "UnRefImageDataVisitor.h"

// --- OSG Stuff --- //
#include <osg/Geode>
#include <osg/Group>
#include <osg/Texture>
#include <osg/TexEnv>

using namespace ves::xplorer::scenegraph::util;

////////////////////////////////////////////////////////////////////////////////
UnRefImageDataVisitor::UnRefImageDataVisitor( osg::Node* osg_node )
    :
    NodeVisitor( TRAVERSE_ALL_CHILDREN )
{
    osg_node->accept( *this );
}
////////////////////////////////////////////////////////////////////////////////
UnRefImageDataVisitor::~UnRefImageDataVisitor()
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
void UnRefImageDataVisitor::apply( osg::Geode& node )
{    
    CheckStateSet( node.getStateSet() );

    for( size_t i = 0; i < node.getNumDrawables(); ++i )
    {
        //Stateset for the drawable
        CheckStateSet( node.getDrawable( i )->getStateSet() );
    }
}
////////////////////////////////////////////////////////////////////////////////
void UnRefImageDataVisitor::apply( osg::Group& node )
{
    CheckStateSet( node.getStateSet() );
    
    osg::NodeVisitor::traverse( node );
}
////////////////////////////////////////////////////////////////////////////////
void UnRefImageDataVisitor::CheckStateSet( osg::StateSet* stateSet )
{
    if( !stateSet )
    {
        return;
    }

    //Texture for the stateset
    osg::ref_ptr< osg::Texture > texture;
    ///Loop over the maximum number of hardware texture units
    for( size_t k = 0; k < 32; ++k )
    {
        texture = dynamic_cast< osg::Texture* >( stateSet->
            getTextureAttribute( k, osg::StateAttribute::TEXTURE ) );
        if( texture.valid() )
        {
            texture->setUnRefImageDataAfterApply( false );
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
