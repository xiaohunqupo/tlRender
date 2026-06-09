set(HEADERS
    ThumbnailSystemTest.h)

set(SOURCE
    ThumbnailSystemTest.cpp)

add_library(tlUITest ${SOURCE} ${HEADERS})

target_compile_definitions(tlUITest PUBLIC TLRENDER_SAMPLE_DATA="${PROJECT_SOURCE_DIR}/etc/SampleData")

target_include_directories(tlUITest
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(tlUITest tlUI ftkTestLib)

set_target_properties(tlUITest PROPERTIES FOLDER tests)
