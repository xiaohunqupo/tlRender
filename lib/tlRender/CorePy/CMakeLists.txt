set(HEADERS
    Bindings.h
    Time.h)
set(HEADERS_PRIVATE)
set(SOURCE
    Bindings.cpp
    Time.cpp)

add_library(tlCorePy ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlCorePy tlCore Python3::Python)
set_target_properties(tlCorePy PROPERTIES FOLDER lib)
set_target_properties(tlCorePy PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlCorePy
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlCorePy)
