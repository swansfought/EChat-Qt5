#include "device/desktop.h"

Desktop* Desktop::instance = nullptr;

Desktop::Desktop()
{
    // 多屏操作
    //    screenList = QGuiApplication::screens();
    //    screenCounts = screenList.size();
    //    if (screenCounts > 1) {
    //        for (int i = 0; i < screenCounts; ++i) {
    //            //
    //        }
    //    }

    // 主屏幕操作
    pScreen = QGuiApplication::primaryScreen();
    pAvailableRect = pScreen->availableGeometry();
    pScreenRect = pScreen->geometry();
}

Desktop::~Desktop() {}
