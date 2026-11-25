QT += core gui widgets
CONFIG += c++17

TARGET = agility_test
TEMPLATE = app

include(src/core.pri)
include(src/ui.pri)

SOURCES += src/main.cpp

# Calidad: sin warnings. Tamano: optimizar por espacio y eliminar codigo muerto.
QMAKE_CXXFLAGS += -Wall -Wextra -ffunction-sections -fdata-sections
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Os
QMAKE_LFLAGS_RELEASE += -Wl,--gc-sections -Wl,-s
