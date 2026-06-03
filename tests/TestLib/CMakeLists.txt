set(HEADERS
    ITest.h
    ITestInline.h)

set(SOURCE
    ITest.cpp)

add_library(tlTestLib ${SOURCE} ${HEADERS})

target_compile_definitions(tlTestLib PUBLIC TLRENDER_SAMPLE_DATA="${PROJECT_SOURCE_DIR}/etc/SampleData")

target_include_directories(tlTestLib
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tlTestLib tlCore)

set_target_properties(tlTestLib PROPERTIES FOLDER tests)
