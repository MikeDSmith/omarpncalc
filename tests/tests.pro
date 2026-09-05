QT += core gui testlib
CONFIG += testcase c++17
TEMPLATE = app
TARGET = tst_calculator

INCLUDEPATH += ../src
SOURCES += \
    tst_calculator.cpp \
    ../src/calculator.cpp
HEADERS += \
    ../src/calculator.h
