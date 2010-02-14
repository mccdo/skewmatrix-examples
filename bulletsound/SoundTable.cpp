// Copyright (c) 2009 Skew Matrix Software LLC. All rights reserved.


#include "SoundTable.h"
#include <string>


SoundData::SoundData()
  : _default( true )
{
}
SoundData::~SoundData()
{
}

SoundData&
SoundData::operator=( const SoundData& rhs )
{
    _default = rhs._default;
    _fileName = rhs._fileName;
    return( *this );
}




