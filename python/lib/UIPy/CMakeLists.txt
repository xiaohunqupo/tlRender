set(HEADERS
    Bindings.h
    PlaybackButtons.h
    TimeEdit.h
    TimelineWidget.h
    Viewport.h)
set(HEADERS_PRIVATE)
set(SOURCE
    Bindings.cpp
    PlaybackButtons.cpp
    TimeEdit.cpp
    TimelineWidget.cpp
    Viewport.cpp)

add_library(tlUIPy ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlUIPy tlUI Python3::Python)
set_target_properties(tlUIPy PROPERTIES FOLDER python/lib)
set_target_properties(tlUIPy PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUIPy
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUIPy)
