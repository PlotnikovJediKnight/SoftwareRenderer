QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    animation.cpp \
    attributeinterpolation.cpp \
    camera.cpp \
    display.cpp \
    framebuffer.cpp \
    lambertianmodel.cpp \
    lightsource.cpp \
    main.cpp \
    mainwindow.cpp \
    objectfileparser.cpp \
    polygonedge.cpp \
    renderingpipeline.cpp \
    scenedata.cpp \
    viewportpolygonmargins.cpp

HEADERS += \
    animation.h \
    attributeinterpolation.h \
    camera.h \
    display.h \
    framebuffer.h \
    lambertianmodel.h \
    lightsource.h \
    mainwindow.h \
    modelstatus.h \
    objectfileparser.h \
    polygonedge.h \
    renderingpipeline.h \
    scenedata.h \
    viewportpolygonmargins.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += -L"$$PWD/glm"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
