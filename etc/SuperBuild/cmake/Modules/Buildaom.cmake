include(ExternalProject)

set(aom_GIT_REPOSITORY "https://aomedia.googlesource.com/aom.git")
set(aom_GIT_TAG "v3.13.3")

set(aom_DEPS)
if(TLRENDER_NASM)
    list(APPEND aom_DEPS NASM)
endif()

set(aom_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    #-DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_PREFIX}/include
    -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib
    #-DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_PREFIX}/bin
    )
if(TLRENDER_NASM)
    list(APPEND aom_ARGS -DCMAKE_ASM_NASM_COMPILER=${CMAKE_INSTALL_PREFIX}/bin/nasm)
endif()

ExternalProject_Add(
    aom
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/aom
    DEPENDS ${aom_DEPS}
    GIT_REPOSITORY ${aom_GIT_REPOSITORY}
    GIT_TAG ${aom_GIT_TAG}
    CMAKE_ARGS ${aom_ARGS})
