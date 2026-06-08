set(HEADERS)

set(SOURCE
    main.cpp)

add_executable(tl-test ${SOURCE} ${HEADERS})

target_include_directories(tl-test
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tl-test
    tlCoreTest
    tlIOTest
    tlTimelineTest)
if(TLRENDER_QT6)
    target_link_libraries(tl-test tlQtTest)
endif()

set_target_properties(tl-test PROPERTIES FOLDER tests)

add_test(tl-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tl-test${CMAKE_EXECUTABLE_SUFFIX})
