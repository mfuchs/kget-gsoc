set(kget_SRCS
    plasma-kget.cpp
    barchart.cpp
    piegraph.cpp
    transfergraph.cpp
    speedgraph.cpp
)

kde4_add_ui_files(kget_SRCS kgetConfig.ui)
kde4_add_plugin(plasma_applet_kget ${kget_SRCS})
target_link_libraries(plasma_applet_kget ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_kget DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-kget-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})