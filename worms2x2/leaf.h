#pragma once
#include <bits/stdc++.h>
#include <immintrin.h>
#include "../util.h"
#include "field.h"

struct Leaf2x2 {
    static constexpr int NODIR = Field::NODIR;
    int f00, f01, f10, f11;
    int endx, endy, dir;
    bool hasEnd() const { return dir != NODIR; }

    Leaf2x2() : f00(0), f01(0), f10(0), f11(0), endx(0), endy(0), dir(NODIR) {};

    Leaf2x2(uint32_t index) {
        assert(index&(1u<<31));
        f00 = (index) & 63;
        f01 = (index>> 6) & 63;
        f10 = (index >> 12) & 63;
        f11 = (index >> 18) & 63;
        endx = (index >> 24) & 1;
        endy = (index >> 25) & 1;
        dir = (index >> 26) & 7;
        assert(hasEnd() == bool(index&(1u<<30)));
    }

    uint32_t getIndex() {
        return f00 |
               f01 << 6 |
               f10 << 12 |
               f11 << 18 |
               endx << 24 |
               endy << 25 |
               dir << 26 |
               hasEnd() << 30 |
               1u << 31;
    }

    void write(Field& fld, int x, int y) const {
        fld.field[x+0][y+0] = f00;
        fld.field[x+0][y+1] = f01;
        fld.field[x+1][y+0] = f10;
        fld.field[x+1][y+1] = f11;
        if (hasEnd())
            fld.posx = x + endx, fld.posy = y + endy, fld.dir = dir;
    }

    Leaf2x2 setPos(int x, int y, int dir) const {
        Leaf2x2 node = *this;
        node.endx = x, node.endy = y, node.dir = dir;
        return node;
    }

    static Leaf2x2 read(const Field& fld, int x, int y) {
        Leaf2x2 node;
        node.f00 = fld.field[x+0][y+0];
        node.f01 = fld.field[x+0][y+1];
        node.f10 = fld.field[x+1][y+0];
        node.f11 = fld.field[x+1][y+1];
        int endx = fld.posx - x, endy = fld.posy - y;
        if ((endx == 0 || endx == 1) && (endy == 0 || endy == 1))
            node.endx = endx, node.endy = endy, node.dir = fld.dir;
        return node;
    }
};
