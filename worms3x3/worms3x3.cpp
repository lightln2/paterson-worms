#include <bits/stdc++.h>
#include <immintrin.h>
#include "../util.h"
#include "../argparser.h"
#include "field.h"
#include "leaf.h"
#include "node.h"
#include "nodesmap.h"
using namespace std;

// [27 -> 7.5GB], [28 -> 15GB], [29 -> 30GB],  [30 -> 60GB]
constexpr int MAP_LOG_SIZE = 28;
static_assert(MAP_LOG_SIZE <= 30, "two bits of the 32-bit index are taken by head and is_leaf");

NodesMap Map = NodesMap(MAP_LOG_SIZE);

class HashWorm {
private:
    uint32_t Root = Map.SetInitialPosition(Map.CreateEmptyNode(7));
public:
    void Save(string file, __int128_t steps) {
        {
            SafeFileWriter out(file);
            out.write(Root);
            out.write(steps);
            Map.Write(out);
        }
        cout << "Saved " << file << ": " << Map.NodesCount() << " nodes, " << steps << " steps" << endl;
    }

    void Load(string file) {
        FileReader in(file);
        Root = in.read<uint32_t>();
        auto steps = in.read<__int128_t>();
        Map.Read(in);
        cout << "Loaded " << Map.NodesCount() << " nodes, " << steps << " steps" << endl;
    }

    __int128_t Run(string pattern) {
        Map.Setup(pattern);
        auto steps = Map.StepsCount(Root);
        Save("state", steps);
        return Run();
    }

    void Init(string pattern) {
        Map.Setup(pattern);
        auto steps = Map.StepsCount(Root);
        Save("state", steps);
    }

    __int128_t RunFile(string file) {
        assert(filesystem::exists(file));
        Load(file);
        return Run();
    }

    __int128_t Run() {
        cout << TimeStr() << " Hashworm3x3-leaf2x2+parent, MAP_LOG_SIZE: " << MAP_LOG_SIZE 
             << ", map size: " << Map.MaxNodesCount() << endl;
        auto lastReport = clock() * 1000L / CLOCKS_PER_SEC;
        __int128_t lastSteps = Map.StepsCount(Root);

        while (!Map.IsFinished()) {
            assert(Map.HasHead(Root));
            if (!Map.HasHead(Map.Center(Root))) {
                Root = Map.CreateBiggerNode(Root);
                cout << "Create bigger node, depth=" << Map.CalcLogSize(Root) << endl;
                Root = Map.RebuildRoot(Root);
                auto steps = Map.StepsCount(Root);
                Save("state", steps);
            }
            assert(Map.HasHead(Map.Center(Root)));
            Root = Map.Evolve(Root);
            __int128_t steps = -1;
            if (Map.NodesCount() >= Map.MaxNodesCount() || Map.IsFinished()) {
                Root = Map.RebuildRoot(Root);
                steps = Map.StepsCount(Root);
                Save("state", steps);
            }
            auto newReport = clock() * 1000L / CLOCKS_PER_SEC;
            if (newReport - lastReport > 1000 || Map.IsFinished()) {
                if (steps == -1) steps = Map.StepsCount(Root);
                auto speed = (steps - lastSteps) * 1000L / (newReport - lastReport);
                cout << TimeStr() << " "
                     << "steps=" << steps
                     << " increase=" << steps - lastSteps
                     << " in " << (newReport - lastReport) / 1000 << " sec"
                     << " rate=" << speed << "/sec" << endl;
                lastSteps = steps;
                lastReport = newReport;
            }
        }

        cout << "Finished: steps=" << lastSteps << endl;
        return lastSteps;
    }
};
    
int main(int argc, const char* argv[]) {
    setup_thousands_separator();
    if (argc == 1) {
        cout << "Worms3x3 simulator" << endl;
        cout << "usage: " << argv[0] << " command [args...]" << endl;
        cout << "  " << argv[0] << " run <pattern>" << endl;
        cout << "Init solver with a pattern and save the state:" << endl;
        cout << "  " << argv[0] << " init <pattern>" << endl;
        cout << "Load state from a file and run solver, saving state periodically:" << endl;
        cout << "  " << argv[0] << " load <state>" << endl;
        exit(0);
    }
    argparser args(argc, argv);

    HashWorm worm;

    auto cmd = args.str(0);

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
