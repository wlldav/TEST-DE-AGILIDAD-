# Nucleo de la aplicacion (sin interfaz grafica): compartido por la app y las pruebas.
QT += sql

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/models/person.h \
    $$PWD/models/user.h \
    $$PWD/models/patient.h \
    $$PWD/models/testrecords.h \
    $$PWD/logic/agilityengine.h \
    $$PWD/logic/statistics.h \
    $$PWD/database/databasemanager.h

SOURCES += \
    $$PWD/models/person.cpp \
    $$PWD/logic/agilityengine.cpp \
    $$PWD/logic/statistics.cpp \
    $$PWD/database/databasemanager.cpp
