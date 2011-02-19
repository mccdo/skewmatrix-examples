//
// Copyright (c) 2009 Skew Matrix Software LLC.
// All rights reserved.
//

#include "OptVisitor.h"
#include <osg/Geode>
#include <osg/Geometry>

#include <osg/io_utils>
#include <iostream>
#include <set>


OptVisitor::OptVisitor()
  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
    changeDLtoVBO_( false ),
    changeVBOtoDL_( false ),
    changeDynamicToStatic_( false ),
    changeDAtoDEUI_( false ),
    triangles_( 0 ),
    triFans_( 0 ),
    triStrips_( 0 ),
    newDEUIs_( 0 ),
    DLtoVBO_( 0 ),
    VBOtoDL_( 0 )
{
}
OptVisitor::~OptVisitor()
{
}

void
OptVisitor::apply( osg::Node& node )
{
    if( changeDynamicToStatic_ )
        node.setDataVariance( osg::Object::STATIC );
    traverse( node );
}

void
OptVisitor::apply( osg::Geode& geode )
{
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        osg::Drawable* draw = geode.getDrawable(i);
        if( changeDLtoVBO_ )
        {
            draw->setUseDisplayList( false );
            draw->setUseVertexBufferObjects( true );
            DLtoVBO_++;
        }
        if( changeVBOtoDL_ )
        {
            draw->setUseDisplayList( true );
            draw->setUseVertexBufferObjects( false );
            VBOtoDL_++;
        }
        if( changeDynamicToStatic_ )
        {
            draw->setDataVariance( osg::Object::STATIC );
        }

        osg::Geometry* geom = dynamic_cast< osg::Geometry* >( draw );
        if( ( draw != NULL ) && changeDAtoDEUI_ )
        {
            osg::ref_ptr< osg::DrawElementsUInt > deui = new osg::DrawElementsUInt( GL_TRIANGLES );

            unsigned int numPS( geom->getNumPrimitiveSets() );
            while( numPS > 0 )
            {
                numPS--;
                osg::PrimitiveSet* ps( geom->getPrimitiveSet( numPS ) );
                if( ps->getType() == osg::PrimitiveSet::DrawArraysPrimitiveType )
                {
                    osg::DrawArrays* da = dynamic_cast< osg::DrawArrays* >( ps );
                    if( ps->getMode() == osg::PrimitiveSet::TRIANGLES )
                    {
                        processTriangles( *da, *deui );
                        geom->removePrimitiveSet( numPS );
                    }
                    else if( ps->getMode() == osg::PrimitiveSet::TRIANGLE_FAN )
                    {
                        processTriFan( *da, *deui );
                        geom->removePrimitiveSet( numPS );
                    }
                    else if( ps->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP )
                    {
                        processTriStrip( *da, *deui );
                        geom->removePrimitiveSet( numPS );
                    }
                }
            }

            // Create the new DEUI.
            if( deui->size() > 0 )
            {
                geom->addPrimitiveSet( deui.get() );
                newDEUIs_++;
            }
        }
    }
}

void
OptVisitor::processTriangles( const osg::DrawArrays& da, osg::VectorGLuint& indices )
{
    ++triangles_;

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first );
    GLsizei processed( 0 );
    while( processed + 3 <= count )
    {
        indices.push_back( index++ );
        indices.push_back( index++ );
        indices.push_back( index++ );
        processed += 3;
    }
}

void
OptVisitor::processTriFan( const osg::DrawArrays& da, osg::VectorGLuint& indices )
{
    ++triFans_;

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first+1 );
    GLsizei processed( 0 );
    while( processed + 3 <= count )
    {
        indices.push_back( first );
        indices.push_back( index++ );
        indices.push_back( index );
        processed++;
    }
}

void
OptVisitor::processTriStrip( const osg::DrawArrays& da, osg::VectorGLuint& indices )
{
    ++triStrips_;

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first );
    GLsizei processed( 0 );
    while( processed + 3 <= count )
    {
        indices.push_back( index );
        indices.push_back( index+1 );
        indices.push_back( index+2 );
        index++;
        processed++;
    }
}

void
OptVisitor::dump( std::ostream& ostr )
{
    ostr << "Converted from DrawArrays to DrawElementsUInt:" << std::endl;
    ostr << "\tTriangles:\t" << triangles_ << std::endl;
    ostr << "\tTriFans:\t" << triFans_ << std::endl;
    ostr << "\tTriStripss:\t" << triStrips_ << std::endl;
    ostr << "Total DrawElementsUInt created: " << newDEUIs_ << std::endl;
    if( changeDLtoVBO_ )
        ostr << "DLs converted to VBOs: " << DLtoVBO_ << std::endl;
    if( changeVBOtoDL_ )
        ostr << "VBOs converted to DLs: " << VBOtoDL_ << std::endl;
}
