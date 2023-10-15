#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include "item/itemchatobject.h"
#include "item/itemcontact.h"
#include "ui/uichat.h"

class ContactInfo
{
public:
    ContactInfo() : isSelect(false), item(nullptr){};
    ~ContactInfo()
    {
        if (nullptr != item) {
            item->deleteLater();
            item = nullptr;
        }
    };
    bool         isSelect;
    ItemContact *item;
};

class ChatObjInfo
{
public:
    ChatObjInfo() : isSelect(false), isFrdId(false), isGrpId(false), item(nullptr), uiChat(nullptr){};
    ~ChatObjInfo()
    {
        if (nullptr != item) {
            item->deleteLater();
            item = nullptr;
        }
        if (nullptr != uiChat) {
            uiChat->deleteLater();
            uiChat = nullptr;
        }
    };
    bool            isSelect;
    bool            isFrdId;
    bool            isGrpId;
    ItemChatObject *item;
    UiChat         *uiChat;
};

#endif  // THUMBNAIL_H
