#pragma once
#include <Windows.h>
#include <wingdi.h>

namespace DrawingApi {

    bool drawBevel(HDC hdc , COLORREF color , LPRECT rc , UINT uType , UINT uFlags);
    void drawTransparentBitmap(HDC hdc , HBITMAP icon , const int xSrcOffset , const int xStart , const int yStart);
    void drawIconSatnHue(HDC hDC , int px , int py , HICON IconHop , int size_x , int size_y , UINT istepIfAniCur , HBRUSH hbrFlickerFreeDraw , UINT diFlags , int apply_satnhue , int saturationValue , int hueIntensity);
}
