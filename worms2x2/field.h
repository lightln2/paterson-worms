#pragma once
#include <bits/stdc++.h>
#include <immintrin.h>
#include "../util.h"
using namespace std;

// Emulates 8x8 square, until worm's head runs outside
struct Field {
    static constexpr int SIZE=10, DIRS=6, MASKS=1<<DIRS, NODIR=7;

    vector<int> rules = vector<int>(MASKS, 0);
    int field[SIZE][SIZE];
    vector<int> pattern;
    int option;
    int posx, posy, dir;

    struct Update { int srcmask, dstmask, dx, dy; };

    static constexpr Update DIR2UPD[DIRS] = {
        {1<<0, 1<<3, 1, 0},
        {1<<1, 1<<4, 1, 1},
        {1<<2, 1<<5, 0, 1},
        {1<<3, 1<<0, -1, 0},
        {1<<4, 1<<1, -1, -1},
        {1<<5, 1<<2, 0, -1},
    };

    int rotate(int mask, int dir) {
        return ((mask >> dir) | (mask << (DIRS - dir))) & (MASKS - 1);
    }

    void setup(string patternStr) {
        for (int i = 0; i < MASKS; i++) rules[i] = NODIR;
        pattern.clear();
        for (auto c: patternStr) pattern.push_back(c - '0');
        option = 0, posx = -1, posy = -1, dir = NODIR;
    }

    void reset() { posx = 0, posy = 0, dir = NODIR; }

    bool finished() { return field[posx][posy] == 0b111111; }

    bool on_edge(int x, int y) { return x == 0 || x == SIZE-1 || y == 0 || y == SIZE-1; }

    long run() {
        assert(dir != NODIR);
        assert (!on_edge(posx, posy));
        long steps = 0;
        while (!finished()) {
            auto newdir = getNewDir(dir, field[posx][posy]);
            auto& upd = DIR2UPD[newdir];
            if (on_edge(posx + upd.dx, posy + upd.dy)) break;
            steps++;
            dir = newdir;
            field[posx][posy] |= upd.srcmask;
            posx += upd.dx, posy += upd.dy;
            field[posx][posy] |= upd.dstmask;
        }
        return steps;
    }

    void Write(SafeFileWriter& out) {
        out.write(rules);
        out.write(pattern);
        out.write(option);
    }

    void Read(FileReader& in) {
        in.read(rules);
        in.read(pattern);
        option = in.read<int>();
    }

private:
    int getNewDir(int dir, int mask) {
        mask = rotate(mask, dir);
        assert(mask == 0 || (mask & 8));
        if (mask == 0) return dir;
        if (popcount(uint32_t(mask)) == 5)
            return (dir + __tzcnt_u32(~mask)) % DIRS;
        if (rules[mask] == NODIR) {
            if (option >= pattern.size())
                cout << "ERROR: no rule for " << bitset<6>(mask) << endl;
            assert(option < pattern.size());
            rules[mask] = pattern[option++];
            cout << "rule: " << bitset<6>(mask) << " -> " << rules[mask] << endl;
        }
        return (dir + rules[mask]) % DIRS;
    }
};

