include(ExternalProject)

set(ftk_GIT_REPOSITORY "https://github.com/grizzlypeak3d/feather-tk.git")
set(ftk_GIT_TAG "894bcefc5a3a0c8932a8e891b4d7c8c14a977c7e")

set(ftk_DEPS ZLIB PNG)
set(ftk_ARGS
    -Dftk_API=${ftk_API}
    -Dftk_ZLIB=OFF
    -Dftk_PNG=OFF
    -Dftk_PYTHON=${TLRENDER_PYTHON}
    -Dftk_TESTS=OFF
    -Dftk_EXAMPLES=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    ftk-sbuild
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk-sbuild
    DEPENDS ${ftk_DEPS}
    GIT_REPOSITORY ${ftk_GIT_REPOSITORY}
    GIT_TAG ${ftk_GIT_TAG}
    INSTALL_COMMAND ""
    SOURCE_SUBDIR etc/SuperBuild
    LIST_SEPARATOR |
    CMAKE_ARGS ${ftk_ARGS})

ExternalProject_Add(
    ftk
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk
    DEPENDS ftk-sbuild
    GIT_REPOSITORY ${ftk_GIT_REPOSITORY}
    GIT_TAG ${ftk_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${ftk_ARGS})
