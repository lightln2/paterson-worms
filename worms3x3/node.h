#pragma once
#include <bits/stdc++.h>
using namespace std;

struct Node {
    std::array<uint32_t, 9> sub;

    Node() : sub{0} {}

    Node(std::array<uint32_t, 9> vals) : sub(vals) {}

    size_t hash() const {
        size_t seed = sub[0] +
                      sub[1] * 31 +
                      sub[2] * (31LL*31) +
                      sub[3] * (31LL*31*31) +
                      sub[4] * (31LL*31*31*31) +
                      sub[5] * (31LL*31*31*31*31) +
                      sub[6] * (31LL*31*31*31*31*31) +
                      sub[7] * (31LL*31*31*31*31*31*31) +
                      sub[8] * (31LL*31*31*31*31*31*31*31);
        return (seed >> 25) ^ (seed >> 17) ^ (seed >> 6) ^ (seed << 2) ^ seed;
    }
    
    bool operator== (const Node& other) const {
        return sub == other.sub;
    }
};

static_assert(sizeof(Node) == 36);
