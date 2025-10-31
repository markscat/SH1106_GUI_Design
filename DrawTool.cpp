
//#define debug_move

#ifdef debug_move_DrawTool
#include "DrawTool.h"
#include "oledwidget.h"
/*DrawTool.cpp:: Included header oledwidget.h is not used directly (fix available)*/

#include <algorithm>
#include <cmath>

// Bresenham's line algorithm
void DrawTool::drawLine(int x0, int y0, int x1, int y1, bool on,uint8_t* buffer) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        widget->setPixel(x0, y0, on,buffer);
        /*
         * DrawTool.cpp:15:9: Use of undeclared identifier 'widget'; did you mean 'Qt::Widget'? (fix available)
qnamespace.h:207:9: 'Qt::Widget' declared here
DrawTool.cpp:15:17: Member reference type 'Qt::WindowType' is not a pointer
         */

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void DrawTool::drawRectangle(int x, int y, int w, int h, bool on, bool fill,uint8_t* buffer) {
    int x0 = std::min(x, x + w);
    int y0 = std::min(y, y + h);
    int x1 = std::max(x, x + w);
    int y1 = std::max(y, y + h);

    if (fill) {
        for (int i = y0; i <= y1; ++i) {
            drawLine(x0, i, x1, i, on,buffer);
        }
    } else {
        drawLine(x0, y0, x1, y0, on,buffer);
        drawLine(x0, y1, x1, y1, on,buffer);
        drawLine(x0, y0, x0, y1, on,buffer);
        drawLine(x1, y0, x1, y1, on,buffer);
    }
}


// ↓↓↓↓ 在 oledwidget.cpp 中，用這個【更穩定】的版本替換掉你舊的 drawCircle 函式 ↓↓↓↓

void DrawTool::drawCircle(const QPoint &p1, const QPoint &p2, uint8_t* buffer)
//DrawTool.cpp:45:16: Out-of-line definition of 'drawCircle' does not match any declaration in 'DrawTool'
   // DrawTool.h:9:7: DrawTool defined here
{
    int x0 = std::min(p1.x(), p2.x());
    int y0 = std::min(p1.y(), p2.y());
    int x1 = std::max(p1.x(), p2.x());
    int y1 = std::max(p1.y(), p2.y());

    // 如果寬度或高度為0，則不繪製，避免演算法出錯
    if (x0 == x1 || y0 == y1) {
        return;
    }

    long xc = (x0 + x1) / 2;
    long yc = (y0 + y1) / 2;
    long a = (x1 - x0) / 2;
    long b = (y1 - y0) / 2;

    long a2 = a * a;
    long b2 = b * b;
    long two_a2 = 2 * a2;
    long two_b2 = 2 * b2;

    // Region 1
    long x = 0;
    long y = b;
    long p = b2 - a2 * b + (a2 / 4);

    while (two_b2 * x < two_a2 * y) {
        OLEDWidget::setPixel(xc + x, yc + y, true, buffer);

        OLEDWidget::setPixel(xc - x, yc + y, true, buffer);
        OLEDWidget::setPixel(xc + x, yc - y, true, buffer);
        OLEDWidget::setPixel(xc - x, yc - y, true, buffer);

        x++;
        if (p < 0) {
            p += two_b2 * x + b2;
        } else {
            y--;
            p += two_b2 * x + b2 - two_a2 * y;
        }
    }

    // Region 2
    p = b2 * (x * x + x) + a2 * (y * y - y) - a2 * b2;
    while (y >= 0) {
        OLEDWidget::setPixel(xc + x, yc + y, true, buffer);
        OLEDWidget::setPixel(xc - x, yc + y, true, buffer);
        OLEDWidget::setPixel(xc + x, yc - y, true, buffer);
        OLEDWidget::setPixel(xc - x, yc - y, true, buffer);

        y--;
        if (p > 0) {
            p -= two_a2 * y + a2;
        } else {
            x++;
            p += two_b2 * x - two_a2 * y + a2;
        }
    }
}
#endif
