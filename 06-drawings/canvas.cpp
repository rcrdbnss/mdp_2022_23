#include "canvas.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>


canvas::canvas(int width, int height) :
    width_(width), height_(height),
    ptr_(width*height, ' ')
{}

void canvas::set(int x, int y, char c)
{
    if (x >= 0 && x < width_ && y >= 0 && y < height_)
        ptr_[y*width_ + x] = c;
}

void canvas::line(int x0, int y0, int x1, int y1, char c)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) {  /* loop */
        set(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}

void canvas::rectangle(int x0, int y0, int x1, int y1, char c)
{
    line(x0, y0, x1, y0, c);
    line(x0, y1, x1, y1, c);
    line(x0, y0, x0, y1, c);
    line(x1, y0, x1, y1, c);
}

void canvas::circle(int xm, int ym, int r, char c)
{
    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    do {
        set(xm - x, ym + y, c); /*   I. Quadrant */
        set(xm - y, ym - x, c); /*  II. Quadrant */
        set(xm + x, ym - y, c); /* III. Quadrant */
        set(xm + y, ym + x, c); /*  IV. Quadrant */
        r = err;
        if (r <= y) err += ++y * 2 + 1;           /* e_xy+e_y < 0 */
        if (r > x || err > y) err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);
}

void canvas::out(std::ostream& os)
{
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            os.put(ptr_[y*width_ + x]);
        }
        os.put('\n');
    }
}
