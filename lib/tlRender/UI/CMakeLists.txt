set(HEADERS
    AudioClipItem.h
    GapItem.h
    IBasicItem.h
    IItem.h
    Init.h
    ThumbnailSystem.h
    TimeEdit.h
    TimeLabel.h
    TimelineItem.h
    TimelineWidget.h
    TransitionItem.h
    VideoClipItem.h
    Viewport.h
    Window.h)
set(HEADERS_PRIVATE
    TimelineItemPrivate.h)

set(SOURCE
    AudioClipItem.cpp
    GapItem.cpp
    IBasicItem.cpp
    IItem.cpp
    Init.cpp
    ThumbnailSystem.cpp
    TimeEdit.cpp
    TimeLabel.cpp
    TimelineItem.cpp
    TimelineWidget.cpp
    TransitionItem.cpp
    VideoClipItem.cpp
    Viewport.cpp
    Window.cpp)

add_library(tlUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlUI PUBLIC tlGL ftk::ftkUI)
set_target_properties(tlUI PROPERTIES FOLDER lib)
set_target_properties(tlUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlUI
    EXPORT tlUITargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/UI)
install(
    EXPORT tlUITargets
    FILE tlUITargets.cmake
    DESTINATION "lib/cmake/tlRender"
    NAMESPACE tlRender::)
