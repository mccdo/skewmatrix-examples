// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include "tracker.h"

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

osg::Vec3 HeadTracker::getPosition( const double t )
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
            _posStack.push_back( randPulledVector( (float)( idx + 1 ) ) );
            // Note that if we popped multiple levels, we only need to push
            // one vector, and successive iterations of the for loop will
            // push more vectors until we are back to a full _depth stack.
        }
        sum += _posStack[ idx ];
        levelTime *= .5;
    }

    const double deltaTime = t - _lastTime;
    _lastTime = t;
    _deltaPos = sum * deltaTime;
    _lastPos += _deltaPos;

    return( _lastPos );
}

osg::Vec3 HeadTracker::getDeltaPosition( const double t )
{
    if( t != _lastTime )
        getPosition( t );
    return( _deltaPos );
}

float HeadTracker::randNeg1To1()
{
    const int mask( 0xfff );
    const int r = rand() & mask;
    return( ( (float)( r ) / (float)( mask ) ) * 2.f - 1.f );
}

osg::Vec3 HeadTracker::randPulledVector( const float magnitude ) const
{
    // Generate a random vector
    const osg::Vec3 randVec( randNeg1To1(), randNeg1To1(), randNeg1To1() );

    // Redirect it back to the origin, Pull z twice as hard to reduce head up/down motion.
    const float pull = -.4f / magnitude;
    const osg::Vec3 pullVec( _lastPos[0] * pull, _lastPos[1] * pull, _lastPos[2] * pull * 2. );

    return( randVec + pullVec );
}
