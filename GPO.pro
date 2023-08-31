QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH+= \
        qkdsim_tusur
        SOURCES += \
                main.cpp \
                qkdsim_tusur/ceSerial.cpp \
                qkdsim_tusur/conserial.cpp \
                qkdsim_tusur/debuglogger.cpp \


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    qkdsim_tusur/abstracthardwareapi.h \
    qkdsim_tusur/apitypes.h \
    qkdsim_tusur/ceSerial.h \
    qkdsim_tusur/conserial.h \
    qkdsim_tusur/debuglogger.h \
    qkdsim_tusur/hwetypes.h \
    qkdsim_tusur/logictypes.h
