#ifdef debug_move_DrawTool


#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#pragma once
#include <QPoint>
#include <cstdint>
class OLEDWidget; // ✅ 向前宣告放這裡

class DrawTool {
public:

    void drawCircle(const QPoint &p1, const QPoint &p2, uint8_t* buffer);


    static void drawLine(int x0, int y0, int x1, int y1, bool color, uint8_t *buffer);
    static void drawRectangle(int x, int y, int w, int h, bool color, bool filled, uint8_t *buffer);
    //static void drawCircle(const QPoint &start, const QPoint &end, bool color, uint8_t *buffer);
};
#endif
#endif // DRAWTOOL_H
