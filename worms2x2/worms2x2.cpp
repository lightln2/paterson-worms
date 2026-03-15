#include <bits/stdc++.h>
#include <immintrin.h>
#include "../util.h"
#include "../argparser.h"
#include "field.h"
#include "bmpgen.h"
#include "leaf.h"

using namespace std;

// 27: 6GB, 28: 12GB, 29: 24GB, 30: 48GB
constexpr int MAP_LOG_SIZE = 29;
static_assert(MAP_LOG_SIZE <= 30, "two bits of the 32-bit index are taken by head and is_leaf");

struct HashNode {
    uint32_t TopLeftIndex, TopRightIndex, BottomLeftIndex, BottomRightIndex;

    HashNode() : HashNode(0, 0, 0, 0) {}

    HashNode(uint32_t topLeft, uint32_t topRight, uint32_t bottomLeft, uint32_t bottomRight) 
        : TopLeftIndex(topLeft), TopRightIndex(topRight), BottomLeftIndex(bottomLeft), BottomRightIndex(bottomRight) {}

    size_t hash() const {
        size_t seed = 
            TopLeftIndex*(31*31*31) + TopRightIndex*(31*31) + BottomLeftIndex*(31) + BottomRightIndex;
        return (seed >> 25) ^ (seed >> 17) ^ (seed >> 6) ^ (seed << 2) ^ seed;
    }
    
    bool operator== (const HashNode& other) const {
        return TopLeftIndex == other.TopLeftIndex &&
               TopRightIndex == other.TopRightIndex &&
               BottomLeftIndex == other.BottomLeftIndex &&
               BottomRightIndex == other.BottomRightIndex;
    }

};

struct HashNodeHash {
    std::size_t operator()(const HashNode& node) const {
        return node.hash();
    }
};

#pragma pack(push, 1)
struct Evolved {
    long steps = 0;
    uint index = 0;
};

struct NodeInfo {
    HashNode node;
    Evolved evolved;
};
#pragma pack(pop)

static_assert(sizeof(NodeInfo) == 28);

struct Node;

struct NodesMap {
    Field Fld;
    size_t MaxSize;
    bool finished = false;

private:
    static constexpr uint INDEX_MASK = ~(1u<<30);
    vector<NodeInfo> NodesList;
    vector<uint> Dict;
    uint DictMask;
public:
    NodesMap(int logSize)  : MaxSize(1<<logSize) , DictMask((4u<<logSize) - 1) , Dict(4u<<logSize) {
        NodesList.reserve(MaxSize + MaxSize / 100);
        NodesList.push_back({});
    }

    void Setup(string pattern) { Fld.setup(pattern); }
    long NodesCount() const { return NodesList.size(); }
    const HashNode& node(uint index) const { return NodesList[index&INDEX_MASK].node; }
    bool isLeaf(uint index) const { return index & (1u<<31); }
    int calcLogSize(uint index) const { return isLeaf(index) ? 1 : 1 + calcLogSize(node(index).TopLeftIndex); }
    bool HasEnd(uint index) const { return index & (1u<<30); }
    Node getNode(uint index) const;
    uint index(const HashNode& sub);

    bool HasEvolve(uint nodeIndex) { return NodesList[nodeIndex&INDEX_MASK].evolved.index != 0; }
    Evolved GetEvolve(uint nodeIndex) { return NodesList[nodeIndex&INDEX_MASK].evolved; }
    void SetEvolve(uint nodeIndex, Evolved evolved) { NodesList[nodeIndex&INDEX_MASK].evolved = evolved; }

    Node RebuildRoot(Node root);

    void Clear() {
        NodesList.clear();
        NodesList.push_back({});
        for (size_t i = 0; i < Dict.size(); i++) Dict[i] = 0;
    }

    void Write(SafeFileWriter& out) {
        Fld.Write(out);
        vector<HashNode> nlist;
        nlist.reserve(NodesList.size());
        for (auto& ni: NodesList) {
            nlist.push_back(ni.node);
        }
        out.write(nlist);
    }

    void Read(FileReader& in) {
        Clear();
        Fld.Read(in);
        vector<HashNode> nlist;
        in.read(nlist);
        for (int i = 1; i < nlist.size(); i++) {
            auto nx = index(nlist[i]);
            assert((nx&INDEX_MASK)==i);
        }
    }
};

NodesMap Map = NodesMap(MAP_LOG_SIZE);

struct Node {
    uint Index;
    bool Is4x4() const { return Map.isLeaf(TopLeftIndex()); }
    bool HasEnd() const { return Map.HasEnd(Index); }

    uint TopLeftIndex() const { return Map.node(Index).TopLeftIndex; }
    uint BottomLeftIndex() const { return Map.node(Index).BottomLeftIndex; }
    uint TopRightIndex() const { return Map.node(Index).TopRightIndex; }
    uint BottomRightIndex() const { return Map.node(Index).BottomRightIndex; }

    Node(uint index) : Index(index) {}

    Node(const HashNode& hnode) : Index(Map.index(hnode)) {}

    Node(uint topLeft, uint topRight, uint bottomLeft, uint bottomRight)
        : Index(Map.index(HashNode{topLeft, topRight, bottomLeft, bottomRight})) {}

    static Node CreateEmptyNode(int depth) {
        assert(depth > 1);
        if (depth == 2) {
            auto empty = Leaf2x2().getIndex();
            return Node(empty, empty, empty, empty);
        } else {
            auto empty = CreateEmptyNode(depth - 1).Index;
            return Node(empty, empty, empty, empty);
        }
    }

    int CalcDepth() const {
        uint32_t cur = Index;
        long depth = 1;
        while (!Map.isLeaf(cur)) {
            depth++;
            cur = Map.getNode(cur).TopLeftIndex();
        }
        return depth;
    }

    Node CreateBiggerNode() const {
        int depth = CalcDepth();
        auto empty = CreateEmptyNode(depth-1).Index;
        auto topLeft = Node(empty, empty, empty, TopLeftIndex()).Index;
        auto topRight = Node(empty, empty, TopRightIndex(), empty).Index;
        auto bottomLeft = Node(empty, BottomLeftIndex(), empty, empty).Index;
        auto bottomRight = Node(BottomRightIndex(), empty, empty, empty).Index;
        return Node(topLeft, topRight, bottomLeft, bottomRight);
    }

    Node TopLeft() const { return Map.getNode(TopLeftIndex()); }
    Node TopRight() const { return Map.getNode(TopRightIndex()); }
    Node BottomLeft() const { return Map.getNode(BottomLeftIndex()); }
    Node BottomRight() const { return Map.getNode(BottomRightIndex()); }

    Leaf2x2 TopLeftLeaf() const { return Leaf2x2(TopLeftIndex()); }
    Leaf2x2 TopRightLeaf() const { return Leaf2x2(TopRightIndex()); }
    Leaf2x2 BottomLeftLeaf() const { return Leaf2x2(BottomLeftIndex()); }
    Leaf2x2 BottomRightLeaf() const { return Leaf2x2(BottomRightIndex()); }

    Node Center() const {
        return Node(TopLeft().BottomRightIndex(), TopRight().BottomLeftIndex(), BottomLeft().TopRightIndex(), BottomRight().TopLeftIndex());
    }

    Node SetInitialPosRoot() const {
        assert(!Is4x4());
        return Node(TopLeft().SetInitialPosNested().Index, TopRightIndex(), BottomLeftIndex(), BottomRightIndex());
    }

    Node SetInitialPosNested() const {
        if (Is4x4())
            return Node(TopLeftIndex(), TopRightIndex(), BottomLeftIndex(), BottomRightLeaf().setPos(0, 0, 0).getIndex());
        else
            return Node(TopLeftIndex(), TopRightIndex(), BottomLeftIndex(), BottomRight().SetInitialPosNested().Index);
    }

    Leaf2x2 CenterLeaf() const {
        Map.Fld.reset();
        TopLeftLeaf().write(Map.Fld, 1, 1);
        TopRightLeaf().write(Map.Fld, 1, 3);
        BottomLeftLeaf().write(Map.Fld, 3, 1);
        BottomRightLeaf().write(Map.Fld, 3, 3);
        return Leaf2x2::read(Map.Fld, 1, 1);
    }

    Evolved Evolve8x8() const {
        assert(TopLeft().Is4x4());
        assert(HasEnd());
        Map.Fld.reset();
        auto write4x4 = [&](const Node& node, int x, int y) {
            node.TopLeftLeaf().write(Map.Fld, x, y);
            node.TopRightLeaf().write(Map.Fld, x, y+2);
            node.BottomLeftLeaf().write(Map.Fld, x+2, y);
            node.BottomRightLeaf().write(Map.Fld, x+2, y+2);
        };

        auto read4x4 = [&](int x, int y) {
            auto l00 = Leaf2x2::read(Map.Fld, x, y).getIndex();
            auto l01 = Leaf2x2::read(Map.Fld, x, y+2).getIndex();
            auto l10 = Leaf2x2::read(Map.Fld, x+2, y).getIndex();
            auto l11 = Leaf2x2::read(Map.Fld, x+2, y+2).getIndex();
            return Node(l00, l01, l10, l11);
        };

        write4x4(TopLeft(), 1, 1);
        write4x4(TopRight(), 1, 5);
        write4x4(BottomLeft(), 5, 1);
        write4x4(BottomRight(), 5, 5);

        long steps = Map.Fld.run();
        Map.finished |= Map.Fld.finished();

        auto n00 = read4x4(1, 1).Index;
        auto n01 = read4x4(1, 5).Index;
        auto n10 = read4x4(5, 1).Index;
        auto n11 = read4x4(5, 5).Index;
        auto evolvedIndex = Node(n00, n01, n10, n11).Index;
        assert(Map.HasEnd(evolvedIndex));
        Map.SetEvolve(Index, {steps, evolvedIndex});
        return {steps, evolvedIndex};
    }

    Evolved Evolve(int depth, int rootdepth) const {
        assert(!Is4x4());
        if (Map.HasEvolve(Index)) {
            return Map.GetEvolve(Index);
        }
        if (TopLeft().Is4x4()) {
            assert(depth == 3);
            return Evolve8x8();
        }

        long steps = 0;
        uint grid[8][8];

        auto set2x2 = [&](const Node& node, int x, int y) {
            grid[x][y] = node.TopLeftIndex();
            grid[x][y+1] = node.TopRightIndex();
            grid[x+1][y] = node.BottomLeftIndex();
            grid[x+1][y+1] = node.BottomRightIndex();
        };

        auto set4x4 = [&](const Node& node, int x, int y) {
            set2x2(node.TopLeft(), x, y);
            set2x2(node.TopRight(), x, y+2);
            set2x2(node.BottomLeft(), x+2, y);
            set2x2(node.BottomRight(), x+2, y+2);
        };

        auto set8x8 = [&](const Node& node) {
            set4x4(node.TopLeft(), 0, 0);
            set4x4(node.TopRight(), 0, 4);
            set4x4(node.BottomLeft(), 4, 0);
            set4x4(node.BottomRight(), 4, 4);
        };

        auto get2x2 = [&](int x, int y) {
            return Node(grid[x][y], grid[x][y+1], grid[x+1][y], grid[x+1][y+1]);
        };     

        auto get4x4 = [&](int x, int y) {
            auto topLeft = get2x2(x, y).Index;
            auto topRight = get2x2(x, y+2).Index;
            auto bottomLeft = get2x2(x+2, y).Index;
            auto bottomRight = get2x2(x+2, y+2).Index;
            return Node(topLeft, topRight, bottomLeft, bottomRight);
        };     

        auto get8x8 = [&]() {
            auto topLeft = get4x4(0, 0).Index;
            auto topRight = get4x4(0, 4).Index;
            auto bottomLeft = get4x4(4, 0).Index;
            auto bottomRight = get4x4(4, 4).Index;
            return Node(topLeft, topRight, bottomLeft, bottomRight);
        };

        auto has = [&](int x, int y) {
            return Map.HasEnd(grid[x][y]);
        };

        auto has2x2 = [&](int x, int y) {
            return has(x,y) || has(x,y+1) || has(x+1, y) || has(x+1, y+1);
        };

        auto evolve4x4 = [&](int x, int y) {
            auto ev = get4x4(x, y).Evolve(depth - 1, rootdepth);
            steps += ev.steps;
            // ASSERT(steps < std::numeric_limits<long>::max() / 2);
            auto node = Map.getNode(ev.index);
            set4x4(node, x, y);
        };

        set8x8(*this);

        while (!Map.finished) {
            if (has2x2(3, 3)) evolve4x4(2, 2);
            else if (has2x2(2, 2)) evolve4x4(1, 1);
            else if (has2x2(2, 4)) evolve4x4(1, 3);
            else if (has2x2(4, 2)) evolve4x4(3, 1);
            else if (has2x2(4, 4)) evolve4x4(3, 3);
            else if (has2x2(1, 1)) evolve4x4(0, 0);
            else if (has2x2(1, 3)) evolve4x4(0, 2);
            else if (has2x2(1, 5)) evolve4x4(0, 4);
            else if (has2x2(3, 1)) evolve4x4(2, 0);
            else if (has2x2(3, 5)) evolve4x4(2, 4);
            else if (has2x2(5, 1)) evolve4x4(4, 0);
            else if (has2x2(5, 3)) evolve4x4(4, 2);
            else if (has2x2(5, 5)) evolve4x4(4, 4);
            else break;
            if (Map.NodesCount() >= Map.MaxSize) {
                break;
            }
        }

        auto evolvedNode = get8x8();
        if (Map.NodesCount() < Map.MaxSize && depth < rootdepth - 8)
            Map.SetEvolve(Index, {steps, evolvedNode.Index});

        return {steps, evolvedNode.Index};
    }
};

Node NodesMap::getNode(uint index) const { 
    return Node(index);
}

uint NodesMap::index(const HashNode& sub) {
    auto nodeHash = sub.hash();
    auto hash = nodeHash & DictMask;
    if (Dict[hash] != 0) {
        if (node(Dict[hash]) == sub) return Dict[hash];
        while (true) {
            auto inc = (nodeHash % 17) | 1;
            hash = (hash + inc) & DictMask;
            if (Dict[hash] == 0) break;
            if (node(Dict[hash]) == sub) return Dict[hash];
        }
    }
    uint32_t newindex = NodesList.size();
    // assert(newindex < Map.MaxSize);
    bool hasEnd = HasEnd(sub.TopLeftIndex) || HasEnd(sub.TopRightIndex) || HasEnd(sub.BottomLeftIndex) || HasEnd(sub.BottomRightIndex);
    newindex |= hasEnd << 30;
    NodesList.push_back({sub, {0,0}});
    Dict[hash] = newindex;
    return newindex;
}

Node NodesMap::RebuildRoot(Node root) {
    cout << "Rebuild: " << NodesList.size() << " -> " << flush;
    auto oldSize = NodesList.size();
    vector<HashNode> oldnodes;
    oldnodes.reserve(10000);
    oldnodes.push_back({});
    unordered_map<uint, uint> remap;
    remap.reserve(10000);
    vector<Evolved> oldevolved;
    oldevolved.reserve(10000);
    oldevolved.push_back({});

    auto mapped = [&](uint index) {
        return isLeaf(index) ? index : remap[index];
    };

    auto scan = [&](auto self, uint index, bool addEvolved) -> void {
        if (isLeaf(index)) return;
        if (remap.contains(index)) return;
        self(self, node(index).TopLeftIndex, false);
        self(self, node(index).TopRightIndex, false);
        self(self, node(index).BottomLeftIndex, false);
        self(self, node(index).BottomRightIndex, false);

        if (addEvolved && HasEnd(index) && GetEvolve(index).index != 0) {
            self(self, GetEvolve(index).index, false);
            oldevolved.push_back(GetEvolve(index));
        }
        else oldevolved.push_back({});

        remap[index] = oldnodes.size() | (HasEnd(index)<<30);
        oldnodes.push_back(node(index));
    };

    scan(scan, root.Index, false);

    auto newRootIndex = oldnodes.size() - 1;

    Clear();

    assert(oldnodes.size() == oldevolved.size());

    for (size_t i = 1; i < oldnodes.size(); i++) {
        const HashNode& node = oldnodes[i];
        auto n00 = mapped(node.TopLeftIndex),
             n01 = mapped(node.TopRightIndex),
             n10 = mapped(node.BottomLeftIndex),
             n11 = mapped(node.BottomRightIndex);
        auto newnode = Node(n00, n01, n10, n11);
        if (i == newRootIndex) assert(HasEnd(newnode.Index));
        assert((newnode.Index&INDEX_MASK) == i);
        if (i == newRootIndex) newRootIndex = newnode.Index;
        auto ev = oldevolved[i];
        if (ev.index != 0) {
            SetEvolve(newnode.Index, {ev.steps, remap[ev.index]});
        }
    }

    cout << NodesList.size() << endl;

    assert(newRootIndex&(1U<<30));
    return getNode(newRootIndex);
}

class HashWorm {
private:
    Node Root = Node::CreateEmptyNode(12).SetInitialPosRoot();
    __int128_t steps = 0;
public:
    bool generate_bitmap = false;

    void Save(string file) {
        SafeFileWriter out(file);
        out.write(Root.Index);
        out.write(steps);
        Map.Write(out);
        cout << "Saved " << file << ": " << Map.NodesCount() << " nodes, " << steps << " steps" << endl;
        if (generate_bitmap) {
            bmpgen::bmpgen("state", "bitmap.bmp");
        }
    }

    void Load(string file) {
        FileReader in(file);
        Root = Node(in.read<uint32_t>());
        steps = in.read<__int128_t>();
        Map.Read(in);
        cout << "Loaded " << Map.NodesCount() << " nodes, " << steps << " steps" << endl;
    }

    void Init(string pattern) {
        Map.Setup(pattern);
        Save("state");
    }

    long Run(string pattern) {
        Map.Setup(pattern);
        return Run();
    }

    long RunFile(string file) {
        assert(filesystem::exists(file));
        Load(file);
        return Run();
    }

    long Run() {
        cout << "MAP_LOG_SIZE: " << MAP_LOG_SIZE << endl;
        auto lastReport = clock() * 1000L / CLOCKS_PER_SEC;
        auto lastSteps = steps;

        while (true) {
            assert(Root.HasEnd());
            if (!Root.Center().HasEnd()) {
                Root = Root.CreateBiggerNode();
                cout << "Create bigger node, depth=" << Root.CalcDepth() << endl;
                if (generate_bitmap) {
                    Save("state");
                }
            }
            assert(Root.Center().HasEnd());
            auto evolved = Root.Evolve(Root.CalcDepth(), Root.CalcDepth());
            Root = Map.getNode(evolved.index);
            steps += evolved.steps;
            if (Map.NodesCount() >= Map.MaxSize) {
                Root = Map.RebuildRoot(Root);
                Save("state");
            }
            auto newReport = clock() * 1000L / CLOCKS_PER_SEC;
            if (newReport - lastReport > 10000)
            {
                uint64_t speed = (steps - lastSteps) * 1000L / (newReport - lastReport);
                cout << "steps=" << steps << " step=" << evolved.steps << " rate=" << speed << "/sec" << endl;
                lastReport = newReport;
                lastSteps = steps;
            }
            if (Map.finished) {
                Root = Map.RebuildRoot(Root);
                Save("state");
                break;
            }
        }
        cout << "Finished! steps=" << steps << endl;
        return steps;
    }
};

int main(int argc, const char* argv[]) {
    setup_thousands_separator();

    if (argc == 1) {
        cout << "Worms2x2 simulator" << endl;
        cout << "usage: " << argv[0] << " command [args...]" << endl;
        cout << "  " << argv[0] << " run <pattern> [-bmpgen]" << endl;
        cout << "Init solver with a pattern and save the state:" << endl;
        cout << "  " << argv[0] << " init <pattern>" << endl;
        cout << "Load state from a file and run solver, saving state periodically:" << endl;
        cout << "  " << argv[0] << " load <state> [-bmpgen]" << endl;
        cout << "options:  " << endl;
        cout << "    -bmpgen     generate bitmap.bmp file on each root rebuild and rescale" << endl;
        exit(0);
    }
    argparser args(argc, argv);

    HashWorm worm;

    auto cmd = args.str(0);
    if (args.hasopt("bmpgen")) {
        cout << "bmpgen!!!" << endl;
        worm.generate_bitmap = true;
    }

    if (cmd == "run") {
        string pattern = args.str(1);
        worm.Run(pattern);
    } else if (cmd == "init") {
        string pattern = args.str(1);
        worm.Init(pattern);
    } else if (cmd == "load") {
        string stateFile = args.str(1);
        worm.RunFile(stateFile);
    } else {
        std::cerr << "unknown command: " << cmd << endl;
        exit(1);
    }

}
