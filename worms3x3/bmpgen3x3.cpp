#include <bits/stdc++.h>
#include <immintrin.h>
#include "../util.h"
#include "../bitmap.h"
using namespace std;

consteval size_t power(size_t base, int pow) {
    return pow == 0 ? 1 : base * power(base, pow - 1);
}
constexpr size_t SIZE = power(3, 6);

struct HashNode {
    std::array<uint32_t, 9> sub;
    bool Is6x6() { return sub[0] & (1u<<31); }
    int Cnt6x6() {
        uint32_t mask = (1u<<24)-1;
        return popcount(sub[0]&mask) + popcount(sub[1]&mask) + popcount(sub[2]&mask) + 
               popcount(sub[3]&mask) + popcount(sub[4]&mask) + popcount(sub[5]&mask) + 
               popcount(sub[6]&mask) + popcount(sub[7]&mask) + popcount(sub[8]&mask); 

    }
    bool HasEnd() {
        uint32_t mask = (1u<<24)-1;
        auto he = [&](int i) {
            return popcount(sub[i]&mask) % 2 == 1;
        };
        return he(0) | he(1) | he(2) | 
               he(3) | he(4) | he(5) | 
               he(6) | he(7) | he(8);
    }

    double density() {
        assert(Is6x6());
        return Cnt6x6() / 24.0 / 9.0;
    }
};

uint32_t root;
vector<int> rules, pattern;
__uint128_t steps;
int option;
vector<HashNode> nodes;
unordered_map<uint32_t, double> density;
unordered_map<uint32_t, bool> hasend;
vector<RGB> image(SIZE*SIZE);
constexpr uint32_t INDEX_MASK = ~(1u<<30);
vector<pair<int,int>> odds;

void pix(int x, int y, int r, int g, int b) {
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return;
    image[SIZE*(SIZE-y) + x] = {uint8_t(r), uint8_t(g), uint8_t(b)};
}

double GetDencity(uint32_t node) {
    if (density.contains(node)) return density[node];
    auto& hn = nodes[node&INDEX_MASK];
    if (hn.Is6x6()) return (density[node]=hn.density());
    auto den = (GetDencity(hn.sub[0]) + GetDencity(hn.sub[1]) + GetDencity(hn.sub[2]) +
               GetDencity(hn.sub[3]) + GetDencity(hn.sub[4]) + GetDencity(hn.sub[5]) +
               GetDencity(hn.sub[6]) + GetDencity(hn.sub[7]) + GetDencity(hn.sub[8])) / 9.0;
    density[node] = den;
    return den;
}

bool HasEnd(uint32_t node) {
    if (hasend.contains(node)) return hasend[node];
    auto& hn = nodes[node&INDEX_MASK];
    if (hn.Is6x6()) return (hasend[node]=hn.HasEnd());
    auto he = HasEnd(hn.sub[0]) | HasEnd(hn.sub[1]) | HasEnd(hn.sub[2]) |
              HasEnd(hn.sub[3]) | HasEnd(hn.sub[4]) | HasEnd(hn.sub[5]) |
              HasEnd(hn.sub[6]) | HasEnd(hn.sub[7]) | HasEnd(hn.sub[8]);
    hasend[node] = he;
    return he;
}

void process(uint32_t node, int x, int y, int size) {
    if (size == 1) {
        if (GetDencity(node) > 1) cout << node << " " << GetDencity(node) << endl;
        assert(GetDencity(node) <= 1);
        uint8_t d = uint8_t(GetDencity(node) * 255);
        pix(x, y, d, d, d);
        if (HasEnd(node)) {
            cout << "HasEnd: " << x << " " << y << endl;
            odds.push_back({x, y});
        }
    } else {
        auto& ch = nodes[node&INDEX_MASK].sub;
        auto sz = size / 3;
        process(ch[0], x, y, sz);
        process(ch[1], x, y+sz, sz);
        process(ch[2], x, y+sz+sz, sz);
        process(ch[3], x+sz, y, sz);
        process(ch[4], x+sz, y+sz, sz);
        process(ch[5], x+sz, y+sz+sz, sz);
        process(ch[6], x+sz+sz, y, sz);
        process(ch[7], x+sz+sz, y+sz, sz);
        process(ch[8], x+sz+sz, y+sz+sz, sz);
    }
}

void add_odds() {
    for (auto [x, y]: odds) {
        for (int i = -4; i <= 4; i++) {
            pix(x+i, y, 0, 0, 255);
            pix(x, y+i, 0, 0, 255);
        }

    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        cout << "Generate bitmap from the worms file" << endl;
        cout << "Usage: " << argv[0] << " " << "[filename]" << endl;
        return 0;
    }
    assert(argc == 2);
    FileReader in(argv[1]);
    root = in.read<uint32_t>();
    steps = in.read<__int128_t>();
    in.read(rules);
    in.read(pattern);
    option = in.read<int>();
    in.read(nodes);
    cout << "size: " << nodes.size() << endl;
    root = nodes[root&INDEX_MASK].sub[4];
    process(root, 0, 0, SIZE);
    add_odds();
    save_bitmap("bitmap.bmp", image, SIZE, SIZE);
    cout << GetDencity(root) << endl;
    return 0;
}
