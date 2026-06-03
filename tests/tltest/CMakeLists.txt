set(HEADERS)

set(SOURCE
    main.cpp)

add_executable(tltest ${SOURCE} ${HEADERS})

target_include_directories(tltest
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tltest
    tlCoreTest
    tlIOTest
    tlTimelineTest)
if(TLRENDER_QT6)
    target_link_libraries(tltest tlQtTest)
endif()

set_target_properties(tltest PROPERTIES FOLDER tests)

add_test(tltest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tltest${CMAKE_EXECUTABLE_SUFFIX})
