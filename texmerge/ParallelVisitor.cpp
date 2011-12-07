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

#include "ParallelVisitor.h"
#include <osg/Node>
#include <osg/Group>
#include <osg/Notify>

#include <stdlib.h>



ParallelVisitor::ParallelVisitor( osg::Node* sgA, osg::Node* sgB )
  : _sgA( sgA ),
    _sgB( sgB ),
    _pvcb( NULL )
{
}
ParallelVisitor::~ParallelVisitor()
{
}

void ParallelVisitor::setCallback( ParallelVisitorCallback* pvcb )
{
    _pvcb = pvcb;
}
ParallelVisitor::ParallelVisitorCallback* ParallelVisitor::getCallback() const
{
    return( _pvcb );
}


void ParallelVisitor::traverse()
{
    recurseTraverse( _sgA.get(), _sgB.get() );
}

bool ParallelVisitor::recurseTraverse( osg::Node* nodeA, osg::Node* nodeB )
{
    osg::Group* grpA( nodeA->asGroup() );
    osg::Group* grpB( nodeB->asGroup() );
    if( (grpA == NULL) || (grpB == NULL) )
        return( true );

    unsigned int idx;
    for( idx=0;
         idx < osg::minimum( grpA->getNumChildren(), grpB->getNumChildren() );
         idx++ )
    {
        osg::ref_ptr< osg::Node > childA( grpA->getChild( idx ) );
        osg::ref_ptr< osg::Node > childB( grpB->getChild( idx ) );
        if( _pvcb != NULL )
        {
            const bool retVal( (*_pvcb)( *childA, *childB ) );
        }
    }

    const unsigned int numChildren( osg::minimum( grpA->getNumChildren(), grpB->getNumChildren() ) );
    for( idx=0; idx<numChildren; idx++ )
    {
        recurseTraverse( grpA->getChild( idx ), grpB->getChild( idx ) );
    }
    return( true );
}
