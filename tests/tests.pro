QT += testlib
QT -= gui
CONFIG += console testcase c++17
CONFIG -= app_bundle

TARGET = tst_core
TEMPLATE = app

QMAKE_CXXFLAGS += -Wall -Wextra -Werror

include(../src/core.pri)

SOURCES += tst_core.cpp
