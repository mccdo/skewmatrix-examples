if( DIRECTINPUT_FOUND)
    add_definitions( -DDIRECTINPUT_ENABLED )
endif()

set( CATEGORY Example )
MAKE_EXECUTABLE( ghostchar
    ghostchar.cpp
    world.cpp
    world.h
    character.cpp
    character.h
)
