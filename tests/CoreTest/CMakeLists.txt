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

target_include_directories(tlCoreTest
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tlCoreTest tlCore ftkTestLib)

set_target_properties(tlCoreTest PROPERTIES FOLDER tests)
