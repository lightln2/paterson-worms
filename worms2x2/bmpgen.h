#pragma once
#include <bits/stdc++.h>
#include <immintrin.h>
#include "../bitmap.h"
#include "../util.h"
using namespace std;

namespace bmpgen {

constexpr int SIZE = 512;

struct HashNode {
    uint32_t n00, n01, n10, n11;
    bool Is2x2() { return n00 & (1u<<31); }
    double density() {
        assert(Is2x2());
        uint32_t mask = (1u<<24)-1;
        return (popcount(n00&mask) + popcount(n01&mask) + popcount(n10&mask) + popcount(n11&mask))
                / 24.0 / 4.0;
    }
};

uint32_t root;
vector<int> rules, pattern;
__uint128_t steps;
int option;
vector<HashNode> nodes;
unordered_map<uint32_t, double> density;
vector<RGB> image(SIZE*SIZE);
constexpr uint32_t INDEX_MASK = ~(1u<<30);
double GetDencity(uint32_t node) {
    if (density.contains(node)) return density[node];
    auto& hn = nodes[node&INDEX_MASK];
    if (hn.Is2x2()) return (density[node]=hn.density());
    else return 
        (density[node] = 
            (GetDencity(hn.n00) + 
             GetDencity(hn.n01) + 
             GetDencity(hn.n10) + 
             GetDencity(hn.n11)) / 4.0);
}

void process(uint32_t node, int x, int y, int size) {
    if (size == 1) {
        if (GetDencity(node) > 1) cout << GetDencity(node) << endl;
        assert(GetDencity(node) <= 1);
        uint8_t d = uint8_t(GetDencity(node) * 255);
        image[SIZE*x + y] = {d, d, d};
    } else {
        auto& hn = nodes[node&INDEX_MASK];
        process(hn.n00, x, y, size/2);
        process(hn.n01, x, y + size/2, size/2);
        process(hn.n10, x + size/2, y, size/2);
        process(hn.n11, x + size/2, y + size/2, size/2);
    }
}

void bmpgen(const std::string& stateFile, const std::string& bmpFile) {
    cout << "Generating bitmap" << stateFile << " -> " << bmpFile << endl;
    FileReader in(stateFile);
    root = in.read<uint32_t>();
    steps = in.read<__int128_t>();
    in.read(rules);
    in.read(pattern);
    option = in.read<int>();
    in.read(nodes);
    process(root, 0, 0, SIZE);
    save_bitmap(bmpFile, image, SIZE, SIZE);
    cout << "Saved; size=" << nodes.size() << "; density=" << GetDencity(root) << endl;
}

} // namespace bmpgen

