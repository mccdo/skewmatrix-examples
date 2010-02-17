// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __SOUND_UTILITIES_H__
#define __SOUND_UTILITIES_H__ 1


#include <osgAudio/SOundState.h>

#include "Material.h"
#include "SoundTable.h"


class SoundUtilities
{
public:
    static SoundUtilities* instance();
    void shutdown();
    ~SoundUtilities();

    // Number of SoundState objects to maintain. Default is -1, which means
    // query the SoundManager and use the number of hardware sounds. Set to 0
    // effectively disables all sound.
    void setCacheSize( int size );

    // Play the soundFile or sample at the fiven position. By default, play
    // it once, or set loop to true.
    osgAudio::SoundState* playSound( const osg::Vec3& pos, const std::string& soundFile, bool loop=false );
    osgAudio::SoundState* playSound( const osg::Vec3& pos, osgAudio::Sample* sample, bool loop=false );

    // Stop the given sound. This is the only way to stop looping sounds.
    // Non-looping sounds stop at the end of the sample, but can also be stopped
    // with this call.
    bool stopSound( osgAudio::SoundState* ss );

    // Collision between two materials.
    void collide( const Material::MaterialType& matA, const Material::MaterialType& matB, const osg::Vec3& pos );

    // One material sliding against the other.
    void slide( const Material::MaterialType& matA, const Material::MaterialType& matB, const osg::Vec3& pos );

    // One material moving.
    void move( const Material::MaterialType& mat, const osg::Vec3& pos );

protected:
    SoundUtilities();

    static SoundUtilities* _s_instance;

    void init();

    // Grow or shrink our list of managed SoundStates according to the cache size.
    void allocateSoundState();

    // Find a SoundState in our cache that is not playing and therefore available.
    osgAudio::SoundState* findFreeSoundState();

    int _cacheSize;

    osgAudio::SoundStateList _ssList;


    // 2D tables to look up sounds by two materials, for colliding or sliding objects.
    SoundTable< Material::MaterialType > _collideTable;
    SoundTable< Material::MaterialType > _slideTable;

    // 1D map to look up sounds by one material, for moving objects.
    SoundTable< Material::MaterialType > _moveTable;
};


// __SOUND_UTILITIES_H__
#endif
