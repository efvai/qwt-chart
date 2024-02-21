QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    qwtchart.cpp \
    qwtchartmagnifier.cpp \
    qwtchartplot.cpp \
    qwtchartscrollbar.cpp \
    qwtcurvepanner.cpp

HEADERS += \
    qwtchart.h \
    qwtchartmagnifier.h \
    qwtchartplot.h \
    qwtchartscrollbar.h \
    qwtcurvepanner.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/../qwt/include
LIBS += "$$PWD/../qwt/lib/qwt.dll"
