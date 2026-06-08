set(HEADERS
    GLFramebufferObject.h
    Init.h)

set(SOURCE
    GLFramebufferObject.cpp
    Init.cpp)

add_library(tlQtQuick ${HEADERS} ${SOURCE})

target_include_directories(tlQtQuick
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tlQtQuick tlQt Qt6::Quick)

set_target_properties(tlQtQuick PROPERTIES FOLDER lib)
set_target_properties(tlQtQuick PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlQtQuick
    EXPORT tlQtQuickTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/QtQuick)
install(
    EXPORT tlQtQuickTargets
    FILE tlQtQuickTargets.cmake
    DESTINATION "lib/cmake/tlRender"
    NAMESPACE tlRender::)
