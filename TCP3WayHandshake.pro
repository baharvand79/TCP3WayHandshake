QT = core multimedia network
CONFIG += c++17 cmdline
greaterThan(QT_MAJOR_VERSION, 5): QT += multimedia
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        tcphandshake.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += C:\boost_1_84_0
LIBS += -LC:\boost_1_84_0\stage\mingw-x64\lib
LIBS += -lws2_32

HEADERS += \
    tcphandshake.h
