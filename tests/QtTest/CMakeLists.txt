set(HEADERS
    TimeObjectTest.h)

set(SOURCE
    TimeObjectTest.cpp)

add_library(tlQtTest ${SOURCE} ${HEADERS})

target_include_directories(tlQtTest
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tlQtTest tlTestLib tlQt)

set_target_properties(tlQtTest PROPERTIES FOLDER tests)
