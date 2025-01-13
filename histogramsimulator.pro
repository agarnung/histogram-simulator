QT += core gui widgets

TEMPLATE = app

CONFIG += c++20

TARGET = HistogramSimulator

SOURCES += \
    main.cpp \
    ./app/bezierinterpolator.cpp \
    ./app/simulator.cpp \
    ./app/simulatorui.cpp

HEADERS += \
    ./app/bezierinterpolator.h \
    ./app/simulator.h \
    ./app/simulatorui.h

FORMS += \
    ./app/simulatorui.ui \

RESOURCES += \
    ./assets/configsHS.qrc

# Enable pkg-config
CONFIG += link_pkgconfig

# Specify the OpenCV package
PKGCONFIG += opencv4

INCLUDEPATH += /usr/local/include/opencv4

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

