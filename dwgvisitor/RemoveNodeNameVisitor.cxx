/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
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
// --- VE-Suite Includes --- //
#include "RemoveNodeNameVisitor.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

#include <boost/regex.hpp>

using namespace ves::xplorer::scenegraph::util;

////////////////////////////////////////////////////////////////////////////////
RemoveNodeNameVisitor::RemoveNodeNameVisitor( osg::Node* node )
    :
    NodeVisitor( TRAVERSE_ALL_CHILDREN )
{
    node->accept( *this );
}
////////////////////////////////////////////////////////////////////////////////
RemoveNodeNameVisitor::RemoveNodeNameVisitor()
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
void RemoveNodeNameVisitor::apply( osg::Node& node )
{
    std::string name = node.getName();

    boost::algorithm::erase_regex( name, boost::regex( "^a_" ) );

    if( boost::regex_search( name, boost::regex( "^unnamed\\ " ) ) )
    {
        name.erase();
        node.setName( name );
        osg::NodeVisitor::traverse( node );
        return;
    }

    if( boost::regex_search( name, boost::regex( "^3D\\ Face\\ " ) ) )
    {
        name.erase();
        node.setName( name );
        osg::NodeVisitor::traverse( node );
        return;
    }

    if( boost::regex_search( name, boost::regex( "^mesh\\ shell\\ " ) ) )
    {
        name.erase();
        node.setName( name );
        osg::NodeVisitor::traverse( node );
        return;
    }

    if( boost::regex_search( name, boost::regex( "^Solid\\ \\(3D\\)\\ " ) ) )
    {
        name.erase();
        node.setName( name );
        osg::NodeVisitor::traverse( node );
        return;
    }

    node.setName( name );
    osg::NodeVisitor::traverse( node );
    return;
}
////////////////////////////////////////////////////////////////////////////////
