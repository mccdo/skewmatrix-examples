//
// Copyright (c) 2008-2009 Skew Matrix  Software LLC.
// All rights reserved.
//

#include "CountsVisitor.h"
#include <osg/LOD>
#include <osg/PagedLOD>
#include <osg/Switch>
#include <osg/Sequence>
#include <osg/Transform>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgSim/DOFTransform>
#include <osgText/Text>
#include <iostream>


CountsVisitor::CountsVisitor( osg::NodeVisitor::TraversalMode mode )
  : osg::NodeVisitor( mode )
{
    reset();
}

CountsVisitor::~CountsVisitor()
{
}

int
CountsVisitor::getVertices() const
{
    return( _vertices );
}

int
CountsVisitor::getDrawArrays() const
{
    return( _drawArrays );
}

void
CountsVisitor::reset()
{
    _depth = 0;
    _maxDepth = 0;

    _nodes = 0;
    _groups = 0;
    _lods = 0;
    _pagedLods = 0;
    _switches = 0;
    _sequences = 0;
    _transforms = 0;
    _matrixTransforms = 0;
    _dofTransforms = 0;
    _geodes = 0;
    _drawables = 0;
    _geometries = 0;
    _nullGeometries = 0;
    _texts = 0;
    _vertices = 0;
    _stateSets = 0;
    _textures = 0;
    _primitiveSets = 0;
    _drawArrays = 0;

    _totalChildren = 0;
    _slowPathGeometries = 0;

    _uNodes.clear();
    _uGroups.clear();
    _uLods.clear();
    _uPagedLods.clear();
    _uSwitches.clear();
    _uSequences.clear();
    _uTransforms.clear();
    _uMatrixTransforms.clear();
    _uDofTransforms.clear();
    _uGeodes.clear();
    _uDrawables.clear();
    _uGeometries.clear();
    _uTexts.clear();
    _uVertices.clear();
    _uStateSets.clear();
    _uTextures.clear();
    _uPrimitiveSets.clear();
    _uDrawArrays.clear();
}

void
CountsVisitor::dump()
{
    std::cout << std::endl;
    std::cout << "      OSG Object \tCount\tUnique" << std::endl;
    std::cout << "      ---------- \t-----\t------" << std::endl;
    std::cout << "           Nodes \t" << _nodes << "\t" << _uNodes.size() << std::endl;
    std::cout << "          Groups \t" << _groups << "\t" << _uGroups.size() << std::endl;
    std::cout << "            LODs \t" << _lods << "\t" << _uLods.size() << std::endl;
    std::cout << "       PagedLODs \t" << _pagedLods << "\t" << _uPagedLods.size() << std::endl;
    std::cout << "        Switches \t" << _switches << "\t" << _uSwitches.size() << std::endl;
    std::cout << "       Sequences \t" << _sequences << "\t" << _uSequences.size() << std::endl;
    std::cout << "      Transforms \t" << _transforms << "\t" << _uTransforms.size() << std::endl;
    std::cout << "MatrixTransforms \t" << _matrixTransforms << "\t" << _uMatrixTransforms.size() << std::endl;
    std::cout << "   DOFTransforms \t" << _dofTransforms << "\t" << _uDofTransforms.size() << std::endl;
    std::cout << "          Geodes \t" << _geodes << "\t" << _uGeodes.size() << std::endl;
    std::cout << "       Drawables \t" << _drawables << "\t" << _uDrawables.size() << std::endl;
    std::cout << "      Geometries \t" << _geometries << "\t" << _uGeometries.size() << std::endl;
    std::cout << "           Texts \t" << _texts << "\t" << _uTexts.size() << std::endl;
    std::cout << "   PrimitiveSets \t" << _primitiveSets << "\t" << _uPrimitiveSets.size() << std::endl;
    std::cout << "      DrawArrays \t" << _drawArrays << "\t" << _uDrawArrays.size() << std::endl;
    std::cout << " NULL Geometries \t" << _nullGeometries << std::endl;

    if (_slowPathGeometries)
        std::cout << "Slow path Geometries: " << _slowPathGeometries << std::endl;
    float avgChildren = (float)_totalChildren / (float)(_nodes+_groups+_lods+_pagedLods+_switches+_sequences+_transforms+_matrixTransforms+_dofTransforms);
    std::cout << "Average children per node: " << avgChildren << std::endl;

    std::cout << "Total vertices: " << _vertices << std::endl;
    std::cout << "Max depth: " << _maxDepth << std::endl;
}

void
CountsVisitor::apply( osg::Node& node )
{
    _nodes++;
    osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
    _uNodes.insert( rp );

    if (++_depth > _maxDepth)
        _maxDepth = _depth;
    traverse( node );
    _depth--;
}

void
CountsVisitor::apply( osg::Group& node )
{
    _groups++;
    osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
    _uGroups.insert( rp );
    _totalChildren += node.getNumChildren();

    if (++_depth > _maxDepth)
        _maxDepth = _depth;
    traverse( (osg::Node&)node );
    _depth--;
}

void
CountsVisitor::apply( osg::LOD& node )
{
    _lods++;
    osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
    _uLods.insert( rp );
    _totalChildren += node.getNumChildren();

    if (++_depth > _maxDepth)
        _maxDepth = _depth;
    traverse( (osg::Node&)node );
    _depth--;
}

void
CountsVisitor::apply( osg::PagedLOD& node )
{
    osg::Group* grp = node.getParent(0);
    osg::Group* gPar = NULL;
    if (grp)
        gPar = grp->getParent(0);

    _pagedLods++;
    osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
    _uPagedLods.insert( rp );
    _totalChildren += node.getNumChildren();

    if (++_depth > _maxDepth)
        _maxDepth = _depth;
    traverse( (osg::Node&)node );
    _depth--;
}

void
CountsVisitor::apply( osg::Switch& node )
{
    _switches++;
    osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
    _uSwitches.insert( rp );
    _totalChildren += node.getNumChildren();

    if (++_depth > _maxDepth)
        _maxDepth = _depth;
    traverse( (osg::Node&)node );
    _depth--;
}

void
CountsVisitor::apply( osg::Sequence& node )
{
    _sequences++;
    osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
    _uSequences.insert( rp );
    _totalChildren += node.getNumChildren();

    if (++_depth > _maxDepth)
        _maxDepth = _depth;
    traverse( (osg::Node&)node );
    _depth--;
}

void
CountsVisitor::apply( osg::Transform& node )
{
    if (dynamic_cast<osgSim::DOFTransform*>( &node ) != NULL)
    {
        _dofTransforms++;
        osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
        _uDofTransforms.insert( rp );
    }
    else
    {
        _transforms++;
        osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
        _uTransforms.insert( rp );
    }
    _totalChildren += node.getNumChildren();

    if (++_depth > _maxDepth)
        _maxDepth = _depth;
    traverse( (osg::Node&)node );
    _depth--;
}

void
CountsVisitor::apply( osg::MatrixTransform& node )
{
    _matrixTransforms++;
    osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
    _uMatrixTransforms.insert( rp );
    _totalChildren += node.getNumChildren();

    if (++_depth > _maxDepth)
        _maxDepth = _depth;
    traverse( (osg::Node&)node );
    _depth--;
}

void
CountsVisitor::apply( osg::Geode& node )
{
    _geodes++;
    osg::ref_ptr<osg::Object> rp = (osg::Object*)&node;
    _uGeodes.insert( rp );

    unsigned int idx;
    for (idx=0; idx<node.getNumDrawables(); idx++)
    {
        osg::Geometry* geom;
        if (dynamic_cast<osgText::Text*>( node.getDrawable( idx ) ) != NULL)
        {
            _texts++;
            osg::ref_ptr<osg::Object> rp = (osg::Object*)node.getDrawable( idx );
            _uTexts.insert( rp );
        }
        else if ( (geom = dynamic_cast<osg::Geometry*>( node.getDrawable( idx ) )) != NULL)
        {
            _geometries++;
            osg::ref_ptr<osg::Object> rp = (osg::Object*)geom;
            _uGeometries.insert( rp );

            if (!geom->areFastPathsUsed())
                _slowPathGeometries++;

            if (geom->getVertexArray())
                _vertices += geom->getVertexArray()->getNumElements();
            else
                _nullGeometries++;
            osg::ref_ptr<osg::Object> rpv = (osg::Object*)( geom->getVertexArray() );
            _uVertices.insert( rpv );

            if( geom->getNumPrimitiveSets() > 0 )
            {
                _primitiveSets += geom->getNumPrimitiveSets();
                osg::Geometry::PrimitiveSetList& psl = geom->getPrimitiveSetList();
                osg::Geometry::PrimitiveSetList::const_iterator pslit;
                for( pslit = psl.begin(); pslit != psl.end(); pslit++ )
                {
                    osg::ref_ptr<osg::Object> rpps = (osg::Object*)( *pslit );
                    _uPrimitiveSets.insert( rpps );
                    const osg::DrawArrays* da = dynamic_cast< const osg::DrawArrays* >( pslit->get() );
                    if( da )
                    {
                        _drawArrays++;
                        osg::ref_ptr<osg::Object> rpda = (osg::Object*)( da );
                        _uDrawArrays.insert( rpda );
                    }
                }
            }
        }
        else
        {
            _drawables++;
            osg::ref_ptr<osg::Object> rp = (osg::Object*)node.getDrawable( idx );
            _uDrawables.insert( rp );
        }
    }

    if (++_depth > _maxDepth)
        _maxDepth = _depth;
    traverse( (osg::Node&)node );
    _depth--;
}
