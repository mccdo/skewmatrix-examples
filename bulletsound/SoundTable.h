// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __SOUND_TABLE_H__
#define __SOUND_TABLE_H__ 1


#include <osgAudio/SoundManager.h>
#include <osgAudio/Sample.h>

#include <string>
#include <map>


struct SoundData;

template< class T >
class SoundTable {
public:
    SoundTable();
    ~SoundTable();

    void setDefaultSound( std::string& soundFile );

    // 2D table
    void addSound( const T& mat0, const T& mat1, std::string& soundFile );
    osgAudio::Sample* getSound( const T& mat0, const T& mat1 );

    // 1D map
    void addSound( const T& mat, std::string& soundFile );
    osgAudio::Sample* getSound( const T& mat );

protected:
    typedef std::map< T, SoundData > SoundMap;
    typedef std::map< T, SoundMap > Table;

    Table _table;
    SoundMap _map;

    osg::ref_ptr< osgAudio::Sample > _defaultSample;
};


struct SoundData {
    SoundData();
    ~SoundData();

    SoundData& operator=( const SoundData& rhs );

    bool _default;

    std::string _fileName;
    osg::ref_ptr< osgAudio::Sample > _sample;
};


template< class T >
SoundTable< T >::SoundTable()
{
    std::string defaultFileName( "a.wav" );
    setDefaultSound( defaultFileName );
}
template< class T >
SoundTable< T >::~SoundTable()
{
}


template< class T > void
SoundTable< T >::setDefaultSound( std::string& soundFile )
{
    const bool addToCache( true );
    _defaultSample = osgAudio::SoundManager::instance()->getSample( soundFile, addToCache );
}


template< class T > void
SoundTable< T >::addSound( const T& mat0, const T& mat1, std::string& soundFile )
{
    const bool addToCache( true );
    osg::ref_ptr< osgAudio::Sample > sample(
        osgAudio::SoundManager::instance()->getSample( soundFile, addToCache ) );

    SoundData& sd0( _table[ mat0 ][ mat1 ] );
    sd0._default = false;
    sd0._fileName = soundFile;
    sd0._sample = sample;

    // Mirror
    SoundData& sd1( _table[ mat1 ][ mat0 ] );
    sd1._default = false;
    sd1._fileName = soundFile;
    sd1._sample = sample;
}

template< class T > osgAudio::Sample*
SoundTable< T >::getSound( const T& mat0, const T& mat1 )
{
    SoundData& sd( _table[ mat0 ][ mat1 ] );
    if( sd._default )
        return( _defaultSample.get() );
    else
        return( sd._sample.get() );
}


template< class T > void
SoundTable< T >::addSound( const T& mat, std::string& soundFile )
{
    const bool addToCache( true );
    osg::ref_ptr< osgAudio::Sample > sample(
        osgAudio::SoundManager::instance()->getSample( soundFile, addToCache ) );

    SoundData& sd( _map[ mat ] );
    sd._default = false;
    sd._fileName = soundFile;
    sd._sample = sample;
}

template< class T > osgAudio::Sample*
SoundTable< T >::getSound( const T& mat )
{
    SoundData& sd( _map[ mat ] );
    if( sd._default )
        return( _defaultSample.get() );
    else
        return( sd._sample.get() );
}



// __SOUND_TABLE_H__
#endif
