set(HEADERS
    AudioTest.h
    HDRTest.h
    TimeTest.h
    URLTest.h)

set(SOURCE
    AudioTest.cpp
    HDRTest.cpp
    TimeTest.cpp
    URLTest.cpp)

add_library(tlCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlCoreTest tlTestLib)
set_target_properties(tlCoreTest PROPERTIES FOLDER tests)
