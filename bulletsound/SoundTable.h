// Copyright (c) 2009 Skew Matrix Software LLC. All rights reserved.

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

    void addSound( const T& mat0, const T& mat1, std::string& soundFile );
    void setDefaultSound( std::string& soundFile );

    osgAudio::Sample* getSound( const T& mat0, const T& mat1 );

protected:
    typedef std::map< T, SoundData > SoundMap;
    typedef std::map< T, SoundMap > Table;

    Table _table;

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
SoundTable< T >::addSound( const T& mat0, const T& mat1, std::string& soundFile )
{
    const bool addToCache( true );
    osg::ref_ptr< osgAudio::Sample > sample(
        osgAudio::SoundManager::instance()->getSample( soundFile, addToCache ) );

    SoundData& sd( _table[ mat0 ][ mat1 ] );
    sd._default = false;
    sd._fileName = soundFile;
    sd._sample = sample;

    // Mirror
    sd = _table[ mat1 ][ mat0 ];
    sd._default = false;
    sd._fileName = soundFile;
    sd._sample = sample;
}

template< class T > void
SoundTable< T >::setDefaultSound( std::string& soundFile )
{
    const bool addToCache( true );
    _defaultSample = osgAudio::SoundManager::instance()->getSample( soundFile, addToCache );
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



// __SOUND_TABLE_H__
#endif
