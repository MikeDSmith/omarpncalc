QT += core gui qml quick dbus
CONFIG += c++17 release
TARGET = omarpncalc
TEMPLATE = app

HEADERS += \
    src/calculator.h \
    src/hyprland.h \
    src/textscale.h \
    src/theme.h

SOURCES += \
    src/main.cpp \
    src/calculator.cpp \
    src/hyprland.cpp \
    src/textscale.cpp \
    src/theme.cpp

RESOURCES += src/resources.qrc

# `make install` (or INSTALL_ROOT=... for packaging)
target.path = /usr/bin
desktop.files = data/omarpncalc.desktop
desktop.path = /usr/share/applications
icon.files = data/omarpncalc.svg
icon.path = /usr/share/icons/hicolor/scalable/apps
INSTALLS += target desktop icon
