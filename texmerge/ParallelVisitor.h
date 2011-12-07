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

#ifndef __PARALLEL_VISITOR_H__
#define __PARALLEL_VISITOR_H__ 1


#include <osg/ref_ptr>
#include <osg/Node>



/** \brief Simultaneously walks two scene graphs
and executes a callback for each node. */
class ParallelVisitor
{
public:
    ParallelVisitor( osg::Node* sgA, osg::Node* sgB );
    ~ParallelVisitor();

    void traverse();

    /** \brief Callback executed for each node. */
    struct ParallelVisitorCallback
    {
        ParallelVisitorCallback() {}
        virtual ~ParallelVisitorCallback() {}

        virtual bool operator()( osg::Node& grpA, osg::Node& grpB ) = 0;
    };
    void setCallback( ParallelVisitorCallback* pvcb );
    ParallelVisitor::ParallelVisitorCallback* getCallback() const;

protected:
    bool recurseTraverse( osg::Node* nodeA, osg::Node* nodeB );

    osg::ref_ptr< osg::Node > _sgA;
    osg::ref_ptr< osg::Node > _sgB;

    ParallelVisitorCallback* _pvcb;
};


#endif
