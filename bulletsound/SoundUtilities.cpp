// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.


#include "SoundUtilities.h"
#include "SoundTable.h"
#include "Material.h"

#include <osgAudio/SoundManager.h>
#include <osgAudio/SoundState.h>
#include <osgAudio/Sample.h>

#include <osg/Notify>


SoundUtilities* SoundUtilities::_s_instance( NULL );

SoundUtilities*
SoundUtilities::instance()
{
    if( _s_instance == NULL )
        _s_instance = new SoundUtilities;
    return( _s_instance );
}

void
SoundUtilities::shutdown()
{
    if( _s_instance != NULL )
    {
        delete _s_instance;
        _s_instance = NULL;
    }
}


SoundUtilities::SoundUtilities()
  : _cacheSize( -1 )
{
    init();
}
SoundUtilities::~SoundUtilities()
{
}



void
SoundUtilities::setCacheSize( int size )
{
    if( _cacheSize != size )
    {
        _cacheSize = size;
        allocateSoundState();
    }
}

osgAudio::SoundState*
SoundUtilities::playSound( const osg::Vec3& pos, const std::string& soundFile, bool loop )
{
    const bool addToCache( true );
    osg::ref_ptr< osgAudio::Sample > sample(
        osgAudio::SoundManager::instance()->getSample( soundFile, addToCache ) );
    if( !sample.valid() )
    {
        osg::notify( osg::WARN ) << "SoundUtilities: Can't obtain sample for \"" << soundFile << "\"." << std::endl;
        return( NULL );
    }

    return( playSound( pos, sample.get(), loop ) );
}

osgAudio::SoundState*
SoundUtilities::playSound( const osg::Vec3& pos, osgAudio::Sample* sample, bool loop )
{
    osg::ref_ptr< osgAudio::SoundState > ss( findFreeSoundState() );
    if( !ss.valid() )
        return( NULL );

    ss->setPosition( pos );
    ss->setSample( sample );
    ss->setGain( 1.0f );
    ss->setReferenceDistance( 60 );
    ss->setRolloffFactor( 3 );
    ss->setPlay( true );
    ss->setLooping( loop );
    ss->allocateSource( 10, false );

    osgAudio::SoundManager::instance()->addSoundState( ss.get() );
    ss->apply();

    return( ss.get() );
}

bool
SoundUtilities::stopSound( osgAudio::SoundState* ss )
{
    if( ss->isPlaying() )
    {
        ss->setPlay( false );
        return( true );
    }

    return( false );
}

void
SoundUtilities::collide( const Material::MaterialType& matA, const Material::MaterialType& matB, const osg::Vec3& pos )
{
    osgAudio::Sample* sample( _collideTable.getSound( matA, matB ) );
    playSound( pos, sample );
}

void
SoundUtilities::slide( const Material::MaterialType& matA, const Material::MaterialType& matB, const osg::Vec3& pos )
{
    //osgAudio::Sample* sample( _slideTable.getSound( matA, matB ) );
    //playSound( pos, sample );
}

void
SoundUtilities::move( const Material::MaterialType& mat, const osg::Vec3& pos )
{
    osgAudio::Sample* sample( _moveTable.getSound( mat ) );
    playSound( pos, sample );
}


void
SoundUtilities::init()
{
    allocateSoundState();

    _collideTable.setDefaultSound( std::string("hit_with_frying_pan_y.wav") );
    _collideTable.addSound( Material::CEMENT,
        Material::FLUBBER, std::string("metal_crunch.wav") );
    _collideTable.addSound( Material::CEMENT,
        Material::SILLY_PUTTY, std::string("phasers3.wav") );

    _slideTable.setDefaultSound( std::string("car_skid.wav") );

    _moveTable.addSound( Material::WOOD_DOOR, std::string("door_creak2.wav") );
}

void
SoundUtilities::allocateSoundState()
{
    unsigned int numSources;
    if( _cacheSize >= 0 )
        numSources = (unsigned int) _cacheSize;
    else
        numSources = osgAudio::SoundManager::instance()->getNumSources();

    if( _ssList.size() < numSources )
    {
        while( _ssList.size() < numSources )
            _ssList.push_front( new osgAudio::SoundState );
    }
    else
    {
        unsigned int limit( _ssList.size() );
        while( ( _ssList.size() > numSources ) && ( --limit > 0 ) )
        {
            osg::ref_ptr< osgAudio::SoundState > ss = _ssList.front();
            _ssList.pop_front();
            if( ss->isPlaying() )
                _ssList.push_back( ss );
        }
        if( _ssList.size() > numSources )
        {
            osg::notify( osg::WARN ) << "SoundUtilities: Unable to reduce cache size to " << numSources << "." << std::endl;
            osg::notify( osg::WARN ) << "  Too many actively playing sounds." << std::endl;
        }
    }
}

osgAudio::SoundState*
SoundUtilities::findFreeSoundState()
{
    unsigned int limit( _cacheSize );
    osgAudio::SoundStateList::iterator it;
    for( it=_ssList.begin();
        ( it != _ssList.end() ) && ( --limit > 0 );
        it++ )
    {
        osg::ref_ptr< osgAudio::SoundState > candidate = (*it).get();
        if( !candidate->isPlaying() )
        {
            return( candidate.get() );
        }
        else if( candidate->getLooping() )
        {
            // Move looping sounds to the end if the list.
            it = _ssList.erase( it );
            _ssList.push_back( candidate );
        }
    }

    osg::notify( osg::WARN ) << "SoundUtilities: Can't find a free SoundState." << std::endl;
    return( NULL );
}
