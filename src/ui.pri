# Interfaz grafica (Qt Widgets): compartida por la app y la prueba de GUI.
QT += widgets

HEADERS += \
    $$PWD/ui/agilitywindow.h \
    $$PWD/ui/barchartwidget.h \
    $$PWD/ui/circlebutton.h \
    $$PWD/ui/logindialog.h \
    $$PWD/ui/mainwindow.h \
    $$PWD/ui/patientformdialog.h \
    $$PWD/ui/statsdialog.h \
    $$PWD/ui/userformdialog.h

SOURCES += \
    $$PWD/ui/agilitywindow.cpp \
    $$PWD/ui/barchartwidget.cpp \
    $$PWD/ui/circlebutton.cpp \
    $$PWD/ui/logindialog.cpp \
    $$PWD/ui/mainwindow.cpp \
    $$PWD/ui/patientformdialog.cpp \
    $$PWD/ui/statsdialog.cpp \
    $$PWD/ui/userformdialog.cpp
