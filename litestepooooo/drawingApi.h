#pragma once
#include <Windows.h>
#include <wingdi.h>

namespace DrawingApi {
    // todo: improve the bevels customization as im not a happy bunny abt it.
    bool drawBevel(HDC hdc , COLORREF color , LPRECT rc , UINT uType , UINT uFlags);
    void drawTransparentBitmap(HDC hdc , HBITMAP icon , const int xSrcOffset , const int xStart , const int yStart);
}
