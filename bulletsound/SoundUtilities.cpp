// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.


#include "SoundUtilities.h"
#include "SoundTable.h"
#include "Material.h"
#include "RemoveSoundVisitor.h"

#include <osgAudio/SoundManager.h>
#include <osgAudio/SoundState.h>
#include <osgAudio/SoundUpdateCB.h>
#include <osgAudio/Sample.h>

#include <osg/Notify>
#include <osg/io_utils>


SoundUtilities* SoundUtilities::_s_instance( NULL );

SoundUtilities*
SoundUtilities::instance()
{
    if( _s_instance == NULL )
        _s_instance = new SoundUtilities;
    return( _s_instance );
}

void
SoundUtilities::shutdown( osg::Node* root )
{
    RemoveSoundVisitor rsv;
    root->accept( rsv );

    if( _s_instance != NULL )
    {
        delete _s_instance;
        _s_instance = NULL;
    }
}


SoundUtilities::SoundUtilities()
{
    init();
}
SoundUtilities::~SoundUtilities()
{
}



void
SoundUtilities::playSound( const osg::Vec3& pos, const std::string& soundFile, float gain )
{
    const bool addToCache( true );
    osg::ref_ptr< osgAudio::Sample > sample(
        osgAudio::SoundManager::instance()->getSample( soundFile, addToCache ) );
    if( !sample.valid() )
    {
        osg::notify( osg::WARN ) << "SoundUtilities: Can't obtain sample for \"" << soundFile << "\"." << std::endl;
        return;
    }

    playSound( pos, sample.get(), gain );
}

void
SoundUtilities::playSound( const osg::Vec3& pos, osgAudio::Sample* sample, float gain )
{
    _soundState->setPosition( pos );
    _soundState->setSample( sample );
    _soundState->setPlay( true );
	_soundState->setPitch( 1 );
    _soundState->setGain( gain );

	osgAudio::SoundManager::instance()->pushSoundEvent( _soundState.get() );

    return;
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
    // TBD. Yuck. Don't have a good sound for this, and need to play just
    // one sound per sliding pair.
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
SoundUtilities::addSound( osg::Node* node, const std::string& soundFile, float gain )
{
    const bool addToCache( true );
    osg::ref_ptr< osgAudio::Sample > sample(
        osgAudio::SoundManager::instance()->getSample( soundFile, addToCache ) );
    if( !sample.valid() )
    {
        osg::notify( osg::WARN ) << "SoundUtilities: Can't obtain sample for \"" << soundFile << "\"." << std::endl;
        return;
    }

    addSound( node, sample.get(), gain );
}

void
SoundUtilities::addSound( osg::Node* node, osgAudio::Sample* sample, float gain )
{
    osg::ref_ptr< osgAudio::SoundState > ss( new osgAudio::SoundState );
    if( !ss.valid() )
    {
        osg::notify( osg::WARN ) << "SoundUtilities: Can't allocate _soundState in addSound()." << std::endl;
        return;
    }
    ss->setReferenceDistance( 60 );
    ss->setRolloffFactor( 3 );
    ss->setLooping( true );
    ss->allocateSource( 0 );
    ss->setSample( sample );
    ss->setPlay( true );
    ss->setGain( gain );
    osgAudio::SoundManager::instance()->addSoundState( ss.get() );

    osg::ref_ptr< osgAudio::SoundUpdateCB > callback( new osgAudio::SoundUpdateCB( ss.get() ) );
    node->setUpdateCallback( callback.get() );
}

bool
SoundUtilities::removeSound( osg::Node* node )
{
    if( dynamic_cast< osgAudio::SoundUpdateCB* >( node->getUpdateCallback() ) )
    {
        node->setUpdateCallback( NULL );
        return( true );
    }
    else
        return( false );
}

void
SoundUtilities::setAmbient( const std::string& soundFile, float gain )
{
    osgAudio::SoundState* ambientSoundState(
        osgAudio::SoundManager::instance()->findSoundState( "ambient" ) );
    if( ambientSoundState == NULL )
    {
        const bool addToCache( true );
        osg::ref_ptr< osgAudio::Sample > sample(
            osgAudio::SoundManager::instance()->getSample( soundFile, addToCache ) );
        if( !sample.valid() )
        {
            osg::notify( osg::WARN ) << "SoundUtilities: Can't obtain ambient sample for \"" << soundFile << "\"." << std::endl;
            return;
        }

        ambientSoundState = new osgAudio::SoundState( "ambient" );
        ambientSoundState->allocateSource( 10 );
        ambientSoundState->setSample( sample );
        ambientSoundState->setAmbient( true );
        ambientSoundState->setLooping( true );
        ambientSoundState->setPlay( true );

        osgAudio::SoundManager::instance()->addSoundState( ambientSoundState );
    }

    ambientSoundState->setGain( gain );
}
void
SoundUtilities::setAmbient( bool ambientEnabled )
{
    osgAudio::SoundState* ambientSoundState(
        osgAudio::SoundManager::instance()->findSoundState( "ambient" ) );
    if( ambientSoundState != NULL )
        ambientSoundState->setPlay( ambientEnabled );
}
bool
SoundUtilities::getAmbient() const
{
    osgAudio::SoundState* ambientSoundState(
        osgAudio::SoundManager::instance()->findSoundState( "ambient" ) );
    if( ambientSoundState != NULL )
        return( ambientSoundState->getPlay() );
    else
        return( false );
}


void
SoundUtilities::init()
{
    _soundState = new osgAudio::SoundState;
    if( !_soundState.valid() )
    {
        osg::notify( osg::WARN ) << "SoundUtilities: Can't allocate _soundState in init()." << std::endl;
        return;
    }
    _soundState->setGain( .8f );
    _soundState->setReferenceDistance( 60 );
    _soundState->setRolloffFactor( 3 );
    _soundState->setLooping( false );
    _soundState->setPlay( false );
    osgAudio::SoundManager::instance()->addSoundState( _soundState.get() );


    _collideTable.setDefaultSound( std::string("hit_with_frying_pan_y.wav") );
    _collideTable.addSound( Material::CEMENT,
        Material::FLUBBER, std::string("blip.wav") );
    _collideTable.addSound( Material::CEMENT,
        Material::SILLY_PUTTY, std::string("boing.wav") );

    _slideTable.setDefaultSound( std::string("car_skid.wav") );

    _moveTable.setDefaultSound( std::string( "" ) );
    _moveTable.addSound( Material::WOOD_DOOR, std::string("door_creak2.wav") );
}
