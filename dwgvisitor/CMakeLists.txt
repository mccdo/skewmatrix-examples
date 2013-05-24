SET( CATEGORY Example )
if(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    MAKE_EXECUTABLE( dwgvisitor
        CompressSubgraphVisitor.cpp
        CompressSubgraphVisitor.h
        RemoveNodeNameVisitor.cxx
        RemoveNodeNameVisitor.h
        main.cpp
    )
    TARGET_LINK_LIBRARIES(dwgvisitor ${Boost_LIBRARIES})
endif()
