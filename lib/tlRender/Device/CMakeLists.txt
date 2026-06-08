set(HEADERS
    Init.h)
set(HEADERS_PRIVATE)

set(SOURCE
    Init.cpp)

if(TLRENDER_BMD)

    list(APPEND HEADERS
        BMDData.h
        BMDDevicesModel.h
        BMDOutputDevice.h
        BMDSystem.h
        BMDUtil.h)
    list(APPEND HEADERS_PRIVATE
        BMDOutputPrivate.h)
    list(APPEND SOURCE
        BMDData.cpp
        BMDDevicesModel.cpp
        BMDOutputCallback.cpp
        BMDHDRVideoFrame.cpp
        BMDOutputDevice.cpp
        BMDSystem.cpp
        BMDUtil.cpp)

    if(WIN32)
        list(APPEND HEADERS_PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.h
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Win/platform.h)
        list(APPEND SOURCE
            ${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Win/platform.cpp)

        set(MIDL_OUTPUT_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c")
        set(MIDL_OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.h")
        set(MIDL_OUTPUT_TLB "${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.tlb")
        set(MIDL_OUTPUT
            ${MIDL_OUTPUT_SOURCE}
            ${MIDL_OUTPUT_HEADER}
            ${MIDL_OUTPUT_TLB})
        set(DECKLINK_API_IDL "${TLRENDER_BMD_SDK}/Win/include/DeckLinkAPI.idl")
        set(MIDL_MACHINE amd64)
        add_custom_command(
            OUTPUT ${MIDL_OUTPUT}
            DEPENDS ${DECKLINK_API_IDL}
            COMMAND midl /env ${MIDL_MACHINE} /header ${MIDL_OUTPUT_HEADER} ${DECKLINK_API_IDL} /tlb ${MIDL_OUTPUT_TLB}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        set_source_files_properties(${MIDL_OUTPUT} PROPERTIES GENERATED TRUE)
        add_custom_target(MIDL_Target DEPENDS ${MIDL_OUTPUT})
    elseif(APPLE)
        list(APPEND HEADERS_PRIVATE
            ${TLRENDER_BMD_SDK}/Mac/include/DeckLinkAPI.h
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/macOS/platform.h)
        list(APPEND SOURCE
            ${TLRENDER_BMD_SDK}/Mac/include/DeckLinkAPIDispatch.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/macOS/platform.cpp)
    elseif(UNIX)
        list(APPEND HEADERS_PRIVATE
            ${TLRENDER_BMD_SDK}/Linux/include/DeckLinkAPI.h
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Linux/platform.h)
        list(APPEND SOURCE
            ${TLRENDER_BMD_SDK}/Linux/include/DeckLinkAPIDispatch.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Linux/platform.cpp)
    endif()
endif()

add_library(tlDevice ${HEADERS})
target_sources(tlDevice PRIVATE ${HEADERS_PRIVATE} ${SOURCE})

target_include_directories(tlDevice
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tlDevice PUBLIC tlGL)

if(TLRENDER_BMD)
    target_compile_definitions(tlDevice PUBLIC TLRENDER_BMD)
    if(WIN32)
        target_include_directories(tlDevice PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Win
            ${CMAKE_CURRENT_BINARY_DIR})
        target_link_libraries(tlDevice PRIVATE comsuppw.lib)
        add_dependencies(tlDevice MIDL_Target)
    elseif(APPLE)
        target_include_directories(tlDevice PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/macOS
            ${TLRENDER_BMD_SDK}/Mac/include)
    elseif(UNIX)
        target_include_directories(tlDevice PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Linux
            ${TLRENDER_BMD_SDK}/Linux/include)
    endif()
endif()

set_target_properties(tlDevice PROPERTIES FOLDER lib)
set_target_properties(tlDevice PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlDevice
    EXPORT tlDeviceTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/Device)
install(
    EXPORT tlDeviceTargets
    FILE tlDeviceTargets.cmake
    DESTINATION "lib/cmake/tlRender"
    NAMESPACE tlRender::)
