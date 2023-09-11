#include "drawingApi.h"

enum edgeTypes
{
    FLAT = 1,
    RAISEDINNER,
    RAISED,
    SUNKENINNER,
    BUMP,
};

COLORREF shadeColor(COLORREF color, int percent)
{
    int R = GetRValue(color);
    int G = GetGValue(color);
    int B = GetBValue(color);

    R = int(R * ( 100 + percent ) / 100);
    G = int(G * ( 100 + percent ) / 100);
    B = int(B * ( 100 + percent ) / 100);

    R = ( R < 255 ) ? R : 255;
    G = ( G < 255 ) ? G : 255;
    B = ( B < 255 ) ? B : 255;

    return RGB(R, G, B);
}

bool DrawingApi::drawBevel(HDC hdc, COLORREF color, LPRECT rc, UINT uType, UINT uFlags)
{
    COLORREF LTInnerI = NULL;
    COLORREF LTOuterI = NULL;
    COLORREF RBInnerI = NULL;
    COLORREF RBOuterI = NULL;

    HPEN LTInnerPen, LTOuterPen;
    HPEN RBInnerPen, RBOuterPen;
    RECT InnerRect = *rc;
    POINT SavePoint;
    HPEN SavePen;
    int LBpenplus = 0;
    int LTpenplus = 0;
    int RTpenplus = 0;
    int RBpenplus = 0;

    /* Init some vars */
    LTInnerPen = LTOuterPen = RBInnerPen = RBOuterPen = (HPEN)GetStockObject(NULL_PEN);
    SavePen = (HPEN)SelectObject(hdc, LTInnerPen);

    switch(uType) {
        case edgeTypes::RAISEDINNER:
        {
            LTInnerI = NULL;
            LTOuterI = color;
            RBInnerI = NULL;
            RBOuterI = shadeColor(color, -40);
        } break;
        case edgeTypes::RAISED:
        {
            LTInnerI = shadeColor(color, 30);
            LTOuterI = color;
            RBInnerI = shadeColor(color, -40);
            RBOuterI = GetSysColor(21);
        } break;
        case edgeTypes::SUNKENINNER:
        {
            LTInnerI = NULL;
            LTOuterI = shadeColor(color, -40);
            RBInnerI = NULL;
            RBOuterI = color;
        } break;
        // 1001 |  21   |  22   |  22   |  21
        case edgeTypes::BUMP:
        {
            LTInnerI = GetSysColor(21);
            LTOuterI = shadeColor(color, 90);
            RBInnerI = shadeColor(color, 90);
            RBOuterI = GetSysColor(21);
        }
        break;

    }

    if(( uFlags & BF_BOTTOMLEFT ) == BF_BOTTOMLEFT)
        LBpenplus = 1;
    if(( uFlags & BF_TOPRIGHT ) == BF_TOPRIGHT)
        RTpenplus = 1;
    if(( uFlags & BF_BOTTOMRIGHT ) == BF_BOTTOMRIGHT)
        RBpenplus = 1;
    if(( uFlags & BF_TOPLEFT ) == BF_TOPLEFT)
        LTpenplus = 1;

    if(LTInnerI != NULL)
        LTInnerPen = (HPEN)GetStockObject(DC_PEN);
    if(LTOuterI != NULL)
        LTOuterPen = (HPEN)GetStockObject(DC_PEN);
    if(RBInnerI != NULL)
        RBInnerPen = (HPEN)GetStockObject(DC_PEN);
    if(RBOuterI != NULL)
        RBOuterPen = (HPEN)GetStockObject(DC_PEN);

    MoveToEx(hdc, 0, 0, &SavePoint);

    /* Draw the outer edge */
    SelectObject(hdc, LTOuterPen);
    SetDCPenColor(hdc, LTOuterI);
    if(uFlags & BF_TOP) {
        MoveToEx(hdc, InnerRect.left, InnerRect.top, NULL);
        LineTo(hdc, InnerRect.right, InnerRect.top);
    }
    if(uFlags & BF_LEFT) {
        MoveToEx(hdc, InnerRect.left, InnerRect.top, NULL);
        LineTo(hdc, InnerRect.left, InnerRect.bottom);
    }
    SelectObject(hdc, RBOuterPen);
    SetDCPenColor(hdc, RBOuterI);
    if(uFlags & BF_BOTTOM) {
        MoveToEx(hdc, InnerRect.left, InnerRect.bottom - 1, NULL);
        LineTo(hdc, InnerRect.right, InnerRect.bottom - 1);
    }
    if(uFlags & BF_RIGHT) {
        MoveToEx(hdc, InnerRect.right - 1, InnerRect.top, NULL);
        LineTo(hdc, InnerRect.right - 1, InnerRect.bottom);
    }

    /* Draw the inner edge */
    SelectObject(hdc, LTInnerPen);
    SetDCPenColor(hdc, LTInnerI);
    if(uFlags & BF_TOP) {
        MoveToEx(hdc, InnerRect.left + LTpenplus, InnerRect.top + 1, NULL);
        LineTo(hdc, InnerRect.right - RTpenplus, InnerRect.top + 1);
    }
    if(uFlags & BF_LEFT) {
        MoveToEx(hdc, InnerRect.left + 1, InnerRect.top + LTpenplus, NULL);
        LineTo(hdc, InnerRect.left + 1, InnerRect.bottom - LBpenplus);
    }
    SelectObject(hdc, RBInnerPen);
    SetDCPenColor(hdc, RBInnerI);
    if(uFlags & BF_BOTTOM) {
        MoveToEx(hdc, InnerRect.left + LBpenplus, InnerRect.bottom - 2, NULL);
        LineTo(hdc, InnerRect.right - RBpenplus, InnerRect.bottom - 2);
    }
    if(uFlags & BF_RIGHT) {
        MoveToEx(hdc, InnerRect.right - 2, InnerRect.top + RTpenplus, NULL);
        LineTo(hdc, InnerRect.right - 2, InnerRect.bottom - RBpenplus);
    }

    /* Cleanup */
    SelectObject(hdc, SavePen);
    MoveToEx(hdc, SavePoint.x, SavePoint.y, NULL);
    return true;
}


void DrawingApi::drawTransparentBitmap(HDC hdc, HBITMAP icon, const int xSrcOffset, const int xStart, const int yStart)
{
    BITMAP    bm;
    COLORREF  cColor;
    HBITMAP   bmAndBack, bmAndObject, bmAndMem, bmSave;
    HBITMAP   bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
    HDC       hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
    POINT     ptSize;

    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, icon);      // Select the bitmap

    GetObject(icon, sizeof(BITMAP), (LPSTR)&bm);
    ptSize.x = bm.bmWidth;                // Get width of bitmap
    ptSize.y = bm.bmHeight;               // Get height of bitmap
    DPtoLP(hdcTemp, &ptSize, 1);          // Convert from device
    // to logical points
// Create some DCs to hold temporary data

    hdcBack = CreateCompatibleDC(hdc);
    hdcObject = CreateCompatibleDC(hdc);
    hdcMem = CreateCompatibleDC(hdc);
    hdcSave = CreateCompatibleDC(hdc);

    // Create a bitmap for each DC. DCs are required for a number of GDI functions

    // Monochrome DC

    bmAndBack = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    // Monochrome DC

    bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    bmAndMem = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
    bmSave = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

    // Each DC must select a bitmap object to store pixel data

    bmBackOld = (HBITMAP)SelectObject(hdcBack, bmAndBack);
    bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
    bmMemOld = (HBITMAP)SelectObject(hdcMem, bmAndMem);
    bmSaveOld = (HBITMAP)SelectObject(hdcSave, bmSave);

    // Set proper mapping mode

    SetMapMode(hdcTemp, GetMapMode(hdc));

    // Save the bitmap sent here, because it will be overwritten.

    BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    // Set the background color of the source DC to the color.
    // contained in the parts of the bitmap that should be transparent

    cColor = SetBkColor(hdcTemp, RGB(0, 255, 0));

    // Create the object mask for the bitmap by performing a BitBlt
    // from the source bitmap to a monochrome bitmap.

    BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    // Set the background color of the source DC back to the original color

    SetBkColor(hdcTemp, cColor);

    // Create the inverse of the object mask

    BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);

    // Copy the background of the main DC to the destination

    BitBlt(hdcMem, xSrcOffset, 0, 16, 15, hdc, xStart, yStart, SRCCOPY);

    // Mask out the places where the bitmap will be placed

    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

    // Mask out the transparent colored pixels on the bitmap

    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

    // XOR the bitmap with the background on the destination DC

    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

    // Copy the destination to the screen

    BitBlt(hdc, xStart, yStart, 16, 15, hdcMem, xSrcOffset, 0, SRCCOPY);

    // Place the original bitmap back into the bitmap sent here

    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

    // Delete the memory bitmaps

    DeleteObject(SelectObject(hdcBack, bmBackOld));
    DeleteObject(SelectObject(hdcObject, bmObjectOld));
    DeleteObject(SelectObject(hdcMem, bmMemOld));
    DeleteObject(SelectObject(hdcSave, bmSaveOld));

    // Delete the memory DCs

    DeleteDC(hdcMem);
    DeleteDC(hdcBack);
    DeleteDC(hdcObject);
    DeleteDC(hdcSave);
    DeleteDC(hdcTemp);
}
