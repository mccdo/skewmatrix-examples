// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#ifndef __TANK_DATA_H__
#define __TANK_DATA_H__ 1

#include <osg/ref_ptr>
#include <osg/Node>
#include <osg/Vec3f>
#include <osg/Vec4f>


class TankData
{
public:
    TankData( osg::Node* node=NULL );
    ~TankData();

    void setNode( osg::Node* node );
    osg::Node* getNode();
    const osg::Node* getNode() const;

    /** Up vector. Default is xyz 0, 0, 1. */
    void setUp( const osg::Vec3f& up );
    const osg::Vec3f& getUp() const;

    /** Normalized percent of capacity. Default is 1.0 (100%). */
    void setPercentOfCapacity( float percent );
    float setPercentOfCapacity() const;

    /** Method for coloring the tank fluid. COLOR_OFF (default)
    disables fluid coloring. COLOR_EXPLICIT uses the explicit color.
    COLOR_FLUID_TYPE looks up the fluid color from the fluid map,
    using the fluid type as the map index. */
    typedef enum {
        COLOR_OFF,
        COLOR_EXPLICIT,
        COLOR_FLUID_TYPE
    } ColorMethod;
    void setColorMethod( const ColorMethod& colorMethod );
    const ColorMethod& getColorMethod() const;

    /** Fluid color when color method is set to COLOR_EXPLICIT. Default
    if rgba 0,0,0,1. */
    void setExplicitColor( const osg::Vec4f& explicitColor );
    const osg::Vec4f& getExplicitColor() const;

    /** Used as an index into the fluid map to look up the fluid color
    when the color method is set to COLOR_FLUID_TYPE. Default is "". */
    void setFluidType( const std::string& fluidType );
    const std::string& getFluidType() const;

    /** Specify the fluid color map. This is a (C/C++) pointer so the map can be
    shared by multiple TankData objects. */
    typedef std::map< std::string, osg::Vec4f > FluidTypeColorMap;
    void setFluidMap( FluidTypeColorMap* fluidMap );
    const FluidTypeColorMap* getFluidMap() const;

protected:
    void updateAll();
    void updateColor();

    osg::ref_ptr< osg::Node > _node;

    osg::Vec3f _up;
    float _percent;

    ColorMethod _colorMethod;
    osg::Vec4f _explicitColor;
    std::string _fluidType;
    FluidTypeColorMap* _fluidMap;
};


// __TANK_DATA_H__
#endif
