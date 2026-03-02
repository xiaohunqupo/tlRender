if(TLRENDER_QT6)
    qt6_add_resources(SOURCE player-qtquick.qrc)
endif()

add_executable(player-qtquick App.h App.cpp main.cpp)
target_link_libraries(player-qtquick tlQtQuick)
set_target_properties(player-qtquick PROPERTIES FOLDER examples)
