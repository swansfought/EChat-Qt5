QT       += core gui network sql multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
# resources_big

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS = EChat.ico

SOURCES += \
    db/database.cpp \
    device/config.cpp \
    device/file.cpp \
    device/screenshot.cpp \
    emo/emoji.cpp \
    global.cpp \
    item/headpicture.cpp \
    item/itemcontact.cpp \
    network/client.cpp \
    device/desktop.cpp \
    item/itemchatobject.cpp \
    item/itemfriendapply.cpp \
    main.cpp \
    ui/uichat.cpp \
    ui/uihome.cpp \
    ui/uilogin.cpp \
    ui/uirely.cpp

HEADERS += \
    db/database.h \
    device/config.h \
    device/file.h \
    device/screenshot.h \
    emo/emoji.h \
    global.h \
    item/headpicture.h \
    item/itemcontact.h \
    network/client.h \
    device/desktop.h \
    item/itemchatobject.h \
    item/itemfriendapply.h \
    thumbnail.h \
    ui/uichat.h \
    ui/uihome.h \
    ui/uilogin.h \
    ui/uirely.h

FORMS += \
    emo/emoji.ui \
    item/headpicture.ui \
    item/itemchatobject.ui \
    item/itemcontact.ui \
    item/itemfriendapply.ui \
    ui/uichat.ui \
    ui/uihome.ui \
    ui/uilogin.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc \
