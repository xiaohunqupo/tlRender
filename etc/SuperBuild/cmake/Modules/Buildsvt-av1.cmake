include(ExternalProject)

set(svt-av1_GIT_REPOSITORY "https://gitlab.com/AOMediaCodec/SVT-AV1.git")
set(svt-av1_GIT_TAG "v2.3.0")

set(svt-av1_DEPS)
if(NOT WIN32)
    list(APPEND svt-av1_DEPS NASM)
endif()

set(svt-av1_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -DBUILD_APPS=OFF
    -DCMAKE_ASM_NASM_COMPILER=${CMAKE_INSTALL_PREFIX}/bin/nasm
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_PREFIX}/include
    -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib
    -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_PREFIX}/bin)

ExternalProject_Add(
    svt-av1
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/svt-av1
    DEPENDS ${svt-av1_DEPS}
    GIT_REPOSITORY ${svt-av1_GIT_REPOSITORY}
    GIT_TAG ${svt-av1_GIT_TAG}
    CMAKE_ARGS ${svt-av1_ARGS})
