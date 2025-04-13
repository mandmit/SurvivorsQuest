QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
#QMAKE_PROJECT_DEPTH = 0
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DeviceController.cpp \
    JsonFieldMediator.cpp \
    PlayerEntry.cpp \
    PlayerInfoWindow.cpp \
    PromptDialog.cpp \
    PublicIpAddressFetcher.cpp \
    SessionManager.cpp \
    SessionUtilities.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    DeviceController.h \
    JsonFieldMediator.h \
    PlayerEntry.h \
    PlayerInfoWindow.h \
    PromptDialog.h \
    PublicIpAddressFetcher.h \
    SessionManager.h \
    SessionUtilities.h \
    mainwindow.h

FORMS += \
    PlayerInfoWindow.ui \
    mainwindow.ui

TRANSLATIONS += \
    SurvivorsQuest_uk_UA.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ProjectResources.qrc
