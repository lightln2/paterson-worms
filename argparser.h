#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "util.h"

// very simple argparser that supports running
// executable command [args...]
// todo: support options
struct argparser {
    std::vector<std::string> args;
    std::unordered_map<std::string,std::string> options;

    argparser(int argc, const char* argv[]) {
        for (int i = 1; i < argc; i++) {
            std::string arg(argv[i]);
            if (arg[0] == '-') {
                arg = arg.substr(1);
                auto eqpos = arg.find('=');
                if (eqpos == std::string::npos) {
                    options[arg] = "";
                } else {
                    options[arg.substr(0, eqpos)] = arg.substr(eqpos);
                }
            } else {
                args.push_back(arg);
            }
        }
    }

    size_t size() const { return args.size(); }
    
    std::string str(int index) const {
        assert(index >= 0 && index < size());
        return args[index]; 
    }

    int num(int index) const {
        auto s = str(index);
        return std::stoi(s);
    }

    bool hasopt(const std::string& opt) const {
        return options.contains(opt);
    }

    std::string getopt(const std::string& opt) {
        return options[opt];
    }

    int getoptint(const std::string& opt, int defaultValue) {
        if (!hasopt(opt)) return defaultValue;
        else return std::stoi(getopt(opt));
    }

};