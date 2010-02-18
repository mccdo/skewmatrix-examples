// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __SOUND_UTILITIES_H__
#define __SOUND_UTILITIES_H__ 1


#include <osgAudio/SoundState.h>

#include "Material.h"
#include "SoundTable.h"


class SoundUtilities
{
public:
    static SoundUtilities* instance();
    void shutdown( osg::Node* root );
    ~SoundUtilities();

    // Play the soundFile or sample at the given position.
    void playSound( const osg::Vec3& pos, const std::string& soundFile );
    void playSound( const osg::Vec3& pos, osgAudio::Sample* sample );

    // Collision between two materials.
    void collide( const Material::MaterialType& matA, const Material::MaterialType& matB, const osg::Vec3& pos );

    // One material sliding against the other.
    void slide( const Material::MaterialType& matA, const Material::MaterialType& matB, const osg::Vec3& pos );

    // One material moving.
    void move( const Material::MaterialType& mat, const osg::Vec3& pos );


    // Add a sound to a Node. Sound will loop.
    void addSound( osg::Node* node, const std::string& soundFile );
    void addSound( osg::Node* node, osgAudio::Sample* sample );

    // Remove a sound from a Node.
    bool removeSound( osg::Node* node );

protected:
    SoundUtilities();

    static SoundUtilities* _s_instance;

    void init();

    // For playing one-time sounds.
    osg::ref_ptr< osgAudio::SoundState > _soundState;


    // 2D tables to look up sounds by two materials, for colliding or sliding objects.
    SoundTable< Material::MaterialType > _collideTable;
    SoundTable< Material::MaterialType > _slideTable;

    // 1D map to look up sounds by one material, for moving objects.
    SoundTable< Material::MaterialType > _moveTable;
};


// __SOUND_UTILITIES_H__
#endif
