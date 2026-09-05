QT += core gui qml quick
CONFIG += c++17 release
TARGET = omarpn
TEMPLATE = app

HEADERS += \
    src/calculator.h \
    src/theme.h

SOURCES += \
    src/main.cpp \
    src/calculator.cpp \
    src/theme.cpp

RESOURCES += src/resources.qrc

# `make install` (or INSTALL_ROOT=... for packaging)
target.path = /usr/bin
desktop.files = data/omarpn.desktop
desktop.path = /usr/share/applications
icon.files = data/omarpn.svg
icon.path = /usr/share/icons/hicolor/scalable/apps
INSTALLS += target desktop icon
