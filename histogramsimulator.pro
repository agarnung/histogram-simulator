QT += core gui widgets

TEMPLATE = app

CONFIG += c++20

TARGET = HistogramSimulator

SOURCES += \
    bezierinterpolator.cpp \
    # histogramsimulator.cpp \
    # histogramsimulatorui.cpp \
    main.cpp \
    simulator.cpp \
    simulatorui.cpp

HEADERS += \
    bezierinterpolator.h \
    # histogramsimulator.h \
    # histogramsimulatorui.h \
    simulator.h \
    simulatorui.h

FORMS += \
    simulatorui.ui \
    # histogramsimulatorui.ui

RESOURCES += \
    configsHS.qrc

# Enable pkg-config
CONFIG += link_pkgconfig

# Specify the OpenCV package
PKGCONFIG += opencv4

INCLUDEPATH += /usr/local/include/opencv4

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

