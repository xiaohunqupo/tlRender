set(HEADERS
    ContextObject.h
    Init.h
    MetaTypes.h
    PlayerObject.h
    TimeObject.h
    ToolTipsFilter.h)
set(PRIVATE_HEADERS)

set(SOURCE
    ContextObject.cpp
    Init.cpp
    PlayerObject.cpp
    TimeObject.cpp
    ToolTipsFilter.cpp)

add_library(tlQt ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})

if(TLRENDER_QT6)
    target_compile_definitions(tlQt PUBLIC TLRENDER_QT6)
endif()

target_include_directories(tlQt
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tlQt Qt6::Gui Qt6::OpenGL tlGL)

set_target_properties(tlQt PROPERTIES FOLDER lib)
set_target_properties(tlQt PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlQt
    EXPORT tlQtTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/Qt)
install(
    EXPORT tlQtTargets
    FILE tlQtTargets.cmake
    DESTINATION "lib/cmake/tlRender"
    NAMESPACE tlRender::)
