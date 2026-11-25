QT += testlib widgets
CONFIG += console testcase c++17
CONFIG -= app_bundle

TARGET = tst_gui
TEMPLATE = app

QMAKE_CXXFLAGS += -Wall -Wextra -Werror

include(../src/core.pri)
include(../src/ui.pri)

SOURCES += tst_gui.cpp
