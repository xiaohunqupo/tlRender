include(ExternalProject)

set(dtk_GIT_REPOSITORY "https://github.com/darbyjohnston/dtk.git")
set(dtk_GIT_TAG "fde0fbb706e9ed42b40d7ed149d475de4708df98")

set(dtk_DEPS ZLIB)
set(dtk_ARGS
    -Ddtk_API=${dtk_API}
    -Ddtk_ZLIB=OFF
    -Ddtk_PYTHON=OFF
    -Ddtk_TESTS=OFF
    -Ddtk_EXAMPLES=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    dtk
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/dtk
    DEPENDS ${dtk_DEPS}
    GIT_REPOSITORY ${dtk_GIT_REPOSITORY}
    GIT_TAG ${dtk_GIT_TAG}
    INSTALL_COMMAND ""
    SOURCE_SUBDIR etc/SuperBuild
    LIST_SEPARATOR |
    CMAKE_ARGS ${dtk_ARGS})
