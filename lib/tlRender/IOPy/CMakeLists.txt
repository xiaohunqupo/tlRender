set(HEADERS
    Bindings.h)
set(HEADERS_PRIVATE)
set(SOURCE
    Bindings.cpp
    IO.cpp)

add_library(tlIOPy ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlIOPy tlIO Python3::Python)
set_target_properties(tlIOPy PROPERTIES FOLDER lib)
set_target_properties(tlIOPy PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlIOPy
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlIOPy)
