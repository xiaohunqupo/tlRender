set(HEADERS
    Bindings.h)
set(HEADERS_PRIVATE)
set(SOURCE
    Audio.cpp
    BackgroundOptions.cpp
    Bindings.cpp
    ColorOptions.cpp
    CompareOptions.cpp
    DisplayOptions.cpp
    ForegroundOptions.cpp
    Player.cpp
    PlayerOptions.cpp
    TimeUnits.cpp
    Timeline.cpp
    TimelineOptions.cpp
    Transition.cpp
    Util.cpp
    Video.cpp)

add_library(tlTimelinePy ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlTimelinePy tlTimeline Python3::Python)
set_target_properties(tlTimelinePy PROPERTIES FOLDER lib)
set_target_properties(tlTimelinePy PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimelinePy
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimelinePy)
