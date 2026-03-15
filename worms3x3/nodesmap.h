#pragma once
#include <bits/stdc++.h>
#include "field.h"
#include "leaf.h"
#include "node.h"
using namespace std;

// if true, squares have an additional pointer to last "parent" - bigger square having this as a center
// resulting in ~10% faster simulation
constexpr bool USE_PARENT_CACHING = true;

struct NodeInfo {
    Node node;
    uint32_t evolved = 0, parent = 0;
};

static_assert(sizeof(NodeInfo) == 44);

struct NodesMap {
private:
    static constexpr uint INDEX_MASK = ~HAS_HEAD_MASK;

    Field Fld;
    size_t MaxSize;
    bool finished = false;
    vector<NodeInfo> NodesList;
    vector<uint> Dict;
    uint DictMask;

public:

    NodesMap(int logSize)  : MaxSize(1ull<<logSize) , DictMask((4ull<<logSize) - 1) , Dict(4ull<<logSize) {
        if (logSize == 30) {
            NodesList.reserve(MaxSize);
            MaxSize -= MaxSize / 128;
        } else {
            NodesList.reserve(MaxSize + MaxSize / 128);
        }
        NodesList.push_back({});
    }

    void Setup(string pattern) { Fld.setup(pattern); }

    size_t NodesCount() const { return NodesList.size(); }
    size_t MaxNodesCount() const { return MaxSize; }
    bool IsFinished() const { return finished; }

    NodeInfo& GetNode(uint32_t index) { return NodesList[index&INDEX_MASK]; }
    const NodeInfo& GetNode(uint32_t index) const { return NodesList[index&INDEX_MASK]; }

    bool IsLeaf(uint32_t index) const { return index & IS_LEAF_MASK; }
    bool Is6x6(uint32_t index) const { return IsLeaf(GetNode(index).node.sub[0]); }
    bool HasHead(uint32_t index) const { return index & HAS_HEAD_MASK; }
    uint32_t Center(uint32_t index) const {
        assert(!IsLeaf(index));
        return GetNode(index).node.sub[4];
    }

    int CalcLogSize(uint32_t index) const { 
        return IsLeaf(index) ? 1 : 1 + CalcLogSize(GetNode(index).node.sub[0]);
    }

    bool HasEvolve(uint32_t index) const { return GetNode(index).evolved != 0; }

    uint32_t MakeNode(const Node& node) {
        bool cacheParent = USE_PARENT_CACHING && !IsLeaf(node.sub[4]) && HasHead(node.sub[4]);
        if (cacheParent) {
            auto cachedParent = GetNode(node.sub[4]).parent;
            if (cachedParent != 0 && node == GetNode(cachedParent).node) return cachedParent;
        }
        auto nodeHash = node.hash();
        auto hash = nodeHash & DictMask;
        if (Dict[hash] != 0) {
            if (GetNode(Dict[hash]).node == node) {
                if (cacheParent) GetNode(node.sub[4]).parent = Dict[hash];
                return Dict[hash];
            }
            while (true) {
                auto inc = (nodeHash % 17) | 1;
                hash = (hash + inc) & DictMask;
                if (Dict[hash] == 0) break;
                if (GetNode(Dict[hash]).node == node) {
                    if (cacheParent) GetNode(node.sub[4]).parent = Dict[hash];
                    return Dict[hash];
                }
            }
        }
        uint32_t index = NodesList.size();
        assert(index < (1<<30));
        bool hasHead = HasHead(node.sub[0]) | HasHead(node.sub[1]) | HasHead(node.sub[2]) |
                       HasHead(node.sub[3]) | HasHead(node.sub[4]) | HasHead(node.sub[5]) |
                       HasHead(node.sub[6]) | HasHead(node.sub[7]) | HasHead(node.sub[8]);
        if (hasHead) index |= HAS_HEAD_MASK;
        NodesList.push_back({node, 0, 0});
        Dict[hash] = index;
        if (cacheParent) GetNode(node.sub[4]).parent = index;
        return index;
    }

    vector<Node> CompressRoot(uint32_t root) {
        vector<Node> nodes;
        nodes.reserve(10000);
        nodes.push_back({});
        unordered_map<uint32_t, uint32_t> remap;
        remap.reserve(10000);

        auto mapped = [&](uint32_t index) {
            return IsLeaf(index) ? index : remap[index];
        };

        auto scan = [&](auto self, uint32_t index) -> void {
            if (IsLeaf(index)) return;
            if (remap.contains(index)) return;
            auto& node = GetNode(index).node;

            self(self, node.sub[0]); self(self, node.sub[1]); self(self, node.sub[2]);
            self(self, node.sub[3]); self(self, node.sub[4]); self(self, node.sub[5]);
            self(self, node.sub[6]); self(self, node.sub[7]); self(self, node.sub[8]);

            remap[index] = nodes.size() | (HasHead(index)<<30);
            nodes.push_back({{
                mapped(node.sub[0]), mapped(node.sub[1]), mapped(node.sub[2]),
                mapped(node.sub[3]), mapped(node.sub[4]), mapped(node.sub[5]),
                mapped(node.sub[6]), mapped(node.sub[7]), mapped(node.sub[8]),
            }});
        };

        scan(scan, root);

        return nodes;
    }

    __int128_t StepsCount(uint32_t root) {
        unordered_map<uint32_t, __int128_t> steps;
        steps.reserve(10000);

        auto getSteps = [&](uint32_t index) -> __int128_t {
            return IsLeaf(index) ? Leaf2x2::edgesCount(index) : steps[index];
        };

        auto scan = [&](auto self, uint32_t index) -> void {
            if (IsLeaf(index)) return;
            if (steps.contains(index)) return;
            auto& node = GetNode(index).node;

            self(self, node.sub[0]);
            self(self, node.sub[1]);
            self(self, node.sub[2]);
            self(self, node.sub[3]);
            self(self, node.sub[4]);
            self(self, node.sub[5]);
            self(self, node.sub[6]);
            self(self, node.sub[7]);
            self(self, node.sub[8]);

            steps[index] = getSteps(node.sub[0]) + getSteps(node.sub[1]) + getSteps(node.sub[2]) + 
                           getSteps(node.sub[3]) + getSteps(node.sub[4]) + getSteps(node.sub[5]) + 
                           getSteps(node.sub[6]) + getSteps(node.sub[7]) + getSteps(node.sub[8]);
        };

        scan(scan, root);

        auto stepsCnt = getSteps(root);
        assert(stepsCnt % 2 == 0);
        return stepsCnt / 2;
    }

    uint32_t RebuildRoot(uint32_t root) {
        assert(HasHead(root));
        cout << "Rebuild: " << NodesList.size() << " -> " << flush;
        auto nodes = CompressRoot(root);

        auto newRootIndex = (nodes.size() - 1) | HAS_HEAD_MASK;

        Clear();

        for (size_t i = 1; i < nodes.size(); i++) {
            auto index = MakeNode(nodes[i]);
            assert((index & INDEX_MASK) == i);
            if (i == nodes.size() - 1) assert(index == newRootIndex);
        }

        cout << NodesList.size() << endl;
        return newRootIndex;
    }

    void Clear() {
        NodesList.clear();
        NodesList.push_back({});
        for (size_t i = 0; i < Dict.size(); i++) Dict[i] = 0;
    }

    uint32_t CreateEmptyNode(int depth) {
        assert(depth > 1);
        auto empty = depth == 2 ? Leaf2x2().getIndex() : CreateEmptyNode(depth - 1);
        return MakeNode({{empty, empty, empty, empty, empty, empty, empty, empty, empty}});
    }

    uint32_t CreateBiggerNode(uint32_t index) {
        int depth = CalcLogSize(index);
        auto empty = CreateEmptyNode(depth);
        return MakeNode({{empty, empty, empty, empty, index, empty, empty, empty, empty}});
    }

    uint32_t SetInitialPosition(uint32_t index) {
        auto node{GetNode(index).node};
        if (Is6x6(index))
            node.sub[4] = Leaf2x2(node.sub[4]).setPos(1, 1, 0).getIndex();
        else
            node.sub[4] = SetInitialPosition(node.sub[4]);
        return MakeNode(node);
    }

    void Write(SafeFileWriter& out) {
        Fld.Write(out);
        vector<Node> nlist;
        nlist.reserve(NodesList.size());
        for (auto& ni: NodesList) {
            nlist.push_back(ni.node);
        }
        out.write(nlist);
    }

    void Read(FileReader& in) {
        Clear();
        Fld.Read(in);
        vector<Node> nlist;
        in.read(nlist);
        for (int i = 1; i < nlist.size(); i++) {
            auto nx = MakeNode(nlist[i]);
            assert((nx&INDEX_MASK)==i);
        }
    }

    uint32_t Evolve6x6(uint32_t index) {
        assert(Is6x6(index));
        assert(HasHead(index));
        Fld.reset();
        const auto& node = GetNode(index).node;

        auto write2x2 = [&](uint32_t leaf, int x, int y) {
            Leaf2x2(leaf).write(Fld, x, y);
        };

        auto read2x2 = [&](int x, int y) {
            return Leaf2x2::read(Fld, x, y).getIndex();
        };

        write2x2(node.sub[0], 1, 1);
        write2x2(node.sub[1], 1, 3);
        write2x2(node.sub[2], 1, 5);
        write2x2(node.sub[3], 3, 1);
        write2x2(node.sub[4], 3, 3);
        write2x2(node.sub[5], 3, 5);
        write2x2(node.sub[6], 5, 1);
        write2x2(node.sub[7], 5, 3);
        write2x2(node.sub[8], 5, 5);

        Fld.run();
        finished |= Fld.finished();

        auto evolved = MakeNode({{
            read2x2(1, 1), read2x2(1, 3), read2x2(1, 5),
            read2x2(3, 1), read2x2(3, 3), read2x2(3, 5),
            read2x2(5, 1), read2x2(5, 3), read2x2(5, 5),
        }});
        assert(HasHead(evolved));
        GetNode(index).evolved = evolved;
        return evolved;
    }

    uint32_t Evolve(uint32_t index) {
        if (HasEvolve(index)) return GetNode(index).evolved;
        if (Is6x6(index)) return Evolve6x6(index);

        uint32_t grid[9][9];
        int head_x, head_y;

        auto set3x3 = [&](uint32_t childIndex, int x, int y) {
            auto node = GetNode(childIndex).node;
            grid[x+0][y+0] = node.sub[0];
            grid[x+0][y+1] = node.sub[1];
            grid[x+0][y+2] = node.sub[2];
            grid[x+1][y+0] = node.sub[3];
            grid[x+1][y+1] = node.sub[4];
            grid[x+1][y+2] = node.sub[5];
            grid[x+2][y+0] = node.sub[6];
            grid[x+2][y+1] = node.sub[7];
            grid[x+2][y+2] = node.sub[8];

            if (HasHead(grid[x+0][y+0])) head_x = x, head_y = y;
            if (HasHead(grid[x+0][y+1])) head_x = x, head_y = y+1;
            if (HasHead(grid[x+0][y+2])) head_x = x, head_y = y+2;
            if (HasHead(grid[x+1][y+0])) head_x = x+1, head_y = y;
            if (HasHead(grid[x+1][y+1])) head_x = x+1, head_y = y+1;
            if (HasHead(grid[x+1][y+2])) head_x = x+1, head_y = y+2;
            if (HasHead(grid[x+2][y+0])) head_x = x+2, head_y = y;
            if (HasHead(grid[x+2][y+1])) head_x = x+2, head_y = y+1;
            if (HasHead(grid[x+2][y+2])) head_x = x+2, head_y = y+2;
        };

        auto set9x9 = [&]() {
            auto node = GetNode(index).node;
            set3x3(node.sub[0], 0, 0);
            set3x3(node.sub[1], 0, 3);
            set3x3(node.sub[2], 0, 6);
            set3x3(node.sub[3], 3, 0);
            set3x3(node.sub[4], 3, 3);
            set3x3(node.sub[5], 3, 6);
            set3x3(node.sub[6], 6, 0);
            set3x3(node.sub[7], 6, 3);
            set3x3(node.sub[8], 6, 6);
        };

        auto get3x3 = [&](int x, int y) {
            return MakeNode({{
                grid[x+0][y], grid[x+0][y+1], grid[x+0][y+2],
                grid[x+1][y], grid[x+1][y+1], grid[x+1][y+2],
                grid[x+2][y], grid[x+2][y+1], grid[x+2][y+2],
            }});
        };

        auto get9x9 = [&]() {
            return MakeNode({{
                get3x3(0, 0), get3x3(0, 3), get3x3(0, 6),
                get3x3(3, 0), get3x3(3, 3), get3x3(3, 6),
                get3x3(6, 0), get3x3(6, 3), get3x3(6, 6),
            }});
        };

        auto canEvolve = [&](int x, int y) {
            return x > 0 && x < 8 && y > 0 && y < 8;
        };

        auto evolve3x3 = [&](int x, int y) {
            assert(canEvolve(x, y));
            auto index3x3 = get3x3(x-1, y-1);
            auto evolved = Evolve(index3x3);
            set3x3(evolved, x-1, y-1);
        };

        set9x9();

        while (!finished && NodesCount() < MaxSize && canEvolve(head_x, head_y)) {
            evolve3x3(head_x, head_y);
        }

        auto evolved = get9x9();
        GetNode(index).evolved = evolved;
        return evolved;
    }

};

