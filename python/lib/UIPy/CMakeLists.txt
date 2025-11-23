set(HEADERS
    Bindings.h
    TimelineWidget.h
    Viewport.h
    Window.h)
set(HEADERS_PRIVATE)
set(SOURCE
    Bindings.cpp
    TimelineWidget.cpp
    Viewport.cpp
    Window.cpp)

add_library(tlUIPy ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlUIPy tlUI Python3::Python)
set_target_properties(tlUIPy PROPERTIES FOLDER python/lib)
set_target_properties(tlUIPy PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUIPy
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUIPy)
