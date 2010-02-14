// Copyright (c) 2009 Skew Matrix Software LLC. All rights reserved.

#ifndef __SOUND_TABLE_H__
#define __SOUND_TABLE_H__ 1


#include <string>
#include <map>


struct SoundData;

template< class T >
class SoundTable {
public:
    SoundTable();
    ~SoundTable();

    void addSound( const T& mat0, const T& mat1, std::string& soundFile );

protected:
    typedef std::map< T, SoundData > SoundMap;
    typedef std::map< T, SoundMap > Table;

    Table _table;
};


struct SoundData {
    SoundData();
    ~SoundData();

    SoundData& operator=( const SoundData& rhs );

    bool _default;

    std::string _fileName;
    //Sample* _sample;
};


template< class T >
SoundTable< T >::SoundTable()
{
}
template< class T >
SoundTable< T >::~SoundTable()
{
}

template< class T > void
SoundTable< T >::addSound( const T& mat0, const T& mat1, std::string& soundFile )
{
    SoundData& sd( _table[ mat0 ][ mat1 ] );
    sd._fileName = soundFile;
    // Mirror
    sd = _table[ mat1 ][ mat0 ];
    sd._fileName = soundFile;
}


// __SOUND_TABLE_H__
#endif
