set(HEADERS
    Mesh.h
    OffscreenBuffer.h
    Render.h
    Shader.h
    Texture.h
    Util.h)
set(SOURCE
    Mesh.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Render.cpp
    Shader.cpp
    Texture.cpp
    Util.cpp)

add_library(tlGL ${HEADERS} ${SOURCE})
target_link_libraries(tlGL tlCore tlGlad)
set_target_properties(tlGL PROPERTIES FOLDER lib)
set_target_properties(tlGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlGL
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlGL)