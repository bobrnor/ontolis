CONFIG      += plugin debug_and_release
TARGET      = $$qtLibraryTarget(olswidgetsplugin)
TEMPLATE    = lib

HEADERS     = OLSOntologyGrapthViewPlugin.h OLSOntologyTreeViewPlugin.h OLSOntologyPalleteViewPlugin.h OLSWidgets.h
SOURCES     = OLSOntologyGrapthViewPlugin.cpp OLSOntologyTreeViewPlugin.cpp OLSOntologyPalleteViewPlugin.cpp OLSWidgets.cpp
RESOURCES   = icons.qrc
LIBS        += -L. 

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += designer
} else {
    CONFIG += designer
}

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(olsontologypalleteview.pri)
include(olsontologygrapthview.pri)
include(olsontologytreeview.pri)
