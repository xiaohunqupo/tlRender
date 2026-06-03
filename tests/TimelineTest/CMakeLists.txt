set(HEADERS
    ColorOptionsTest.h
    CompareOptionsTest.h
    DisplayOptionsTest.h
    MemRefTest.h
    PlayerOptionsTest.h
    PlayerTest.h
    TimelineTest.h
    UtilTest.h)

set(SOURCE
    ColorOptionsTest.cpp
    CompareOptionsTest.cpp
    DisplayOptionsTest.cpp
    MemRefTest.cpp
    PlayerOptionsTest.cpp
    PlayerTest.cpp
    TimelineTest.cpp
    UtilTest.cpp)

add_library(tlTimelineTest ${SOURCE} ${HEADERS})

target_include_directories(tlTimelineTest
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tlTimelineTest tlTestLib tlTimeline)

set_target_properties(tlTimelineTest PROPERTIES FOLDER tests)
