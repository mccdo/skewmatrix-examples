SET( CATEGORY Example )
MAKE_EXECUTABLE( bulletsound
    bulletsound.cpp
    Material.cpp
    Material.h
    RemoveSoundVisitor.h
    SoundTable.cpp
    SoundTable.h
    SoundUtilities.cpp
    SoundUtilities.h
)
TARGET_LINK_LIBRARIES( bulletsound ${OSGAUDIO_LIBRARIES})