pybind11_add_module(tlRenderPy Bindings.cpp)
target_link_libraries(tlRenderPy PRIVATE tlUIPy tlTimelinePy tlCorePy)
set_target_properties(tlRenderPy PROPERTIES FOLDER lib)
set_target_properties(tlRenderPy PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlRenderPy
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
