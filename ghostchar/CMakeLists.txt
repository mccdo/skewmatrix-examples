if( DIRECTINPUT_FOUND)
    add_definitions( -DDIRECTINPUT_ENABLED )
endif()

set( CATEGORY Example )

MAKE_EXECUTABLE( ghostchar
    ghostchar.cpp
    character.cpp
    character.h
    tracker.cpp
    tracker.h
    world.cpp
    world.h
    LocalKinematicCharacterController.cpp
    LocalKinematicCharacterController.h
)
