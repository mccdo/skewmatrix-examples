// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include "tracker.h"

#include <osg/Matrix>
#include <osg/Notify>
#include <osg/io_utils>
#include <deque>
#include <math.h>


HeadTracker::HeadTracker( double baseDuration, unsigned int depth )
  : _baseDuration( baseDuration ),
    _depth( depth ),
    _lastTime( 0. )
{
}
HeadTracker::~HeadTracker()
{
}

osg::Vec3 HeadTracker::getMatrix( double t )
{
    osg::Vec3 sum( 0., 0., 0. );
    double levelTime( _baseDuration );
    unsigned int idx;
    for( idx=0; idx<_depth; idx++ )
    {
        // Check to see if the current level's vector has expired
        // (been on the stack too long). It has expired if it's been
        // in the stack for longer than levelTime.
        double aInt, bInt;
        modf( _lastTime / levelTime, &aInt );
        modf( t / levelTime, &bInt );
        if( ( aInt != bInt ) || ( _lastTime == 0. ) )
        {
            while( _posStack.size() > idx )
                _posStack.pop_back();
            // Generate new vector for this stack level. Multiple z by 0.5
            // for more stable head up/down position.
            _posStack.push_back( osg::Vec3( randNeg1To1(), randNeg1To1(), randNeg1To1() * .5 ) );
            // Note that if we popped multiple levels, we only need to push
            // one vector, and successive iterations of the for loop will
            // push more vectors until we are back to a full _depth stack.
        }
        sum += _posStack[ idx ];
        levelTime *= .5;
    }

    double deltaTime = t - _lastTime;
    _lastTime = t;
    _lastPos += ( sum * deltaTime );

    return( sum );
}

float HeadTracker::randNeg1To1()
{
    const int mask( 0xfff );
    const int r = rand() & mask;
    return( ( (float)( r ) / (float)( mask ) ) * 2.f - 1.f );
}

