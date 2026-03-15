#pragma once

#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>


void setup_thousands_separator() {
    struct separate_thousands : std::numpunct<char> {
        char_type do_thousands_sep() const override { return ','; }
        string_type do_grouping() const override { return "\3"; }
    };
    std::cout.imbue(std::locale(std::cout.getloc(), new separate_thousands));    
}

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// standard assert does coredump on errors; this can take minutes in memory-greedy apps.
#define ASSERT(condition) \
do { \
    if(!bool(condition)) { \
        std::cerr << "assert failed: '" << #condition <<  \
            "' in " << __FILENAME__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    } \
} while(0)

#undef assert
#define assert(condition) ASSERT(condition)

decltype(auto) TimeStr() {
    time_t cur_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return std::put_time(std::localtime(&cur_time), "%F %T");
}

std::ostream& operator<<(std::ostream& out, __uint128_t val) {
    if (val == 0) {
        return (out << 0);
    }
    static std::vector<char> buffer;
    buffer.clear();
    while (val) {
        if (buffer.size() % 4 == 3) buffer.push_back(',');
        buffer.push_back('0' + val % 10);
        val /= 10;
    }
    std::reverse(buffer.begin(), buffer.end());
    for (auto c: buffer) out << c;
    return out;
}

std::ostream& operator<<(std::ostream& out, __int128_t val) {
    if (val < 0) {
        out << '-';
        val = -val;
    }
    return out << __uint128_t(val);
}

class FileWriter {
    int fd;
public:
    FileWriter(const std::string& filename) {
        fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        assert(fd != -1);
    }

    template<typename T>
    FileWriter& write(const T& x) {
        char buf[sizeof(T)];
        *(T*)buf = x;
        int res = ::write(fd, buf, sizeof(T));
        assert(res == sizeof(T));
        return *this;
    }

    template<typename T>
    FileWriter& write(const std::vector<T>& x) {
        write<size_t>(x.size());
        int res = ::write(fd, (const char*)&x[0], sizeof(T)*x.size());
        assert(res == sizeof(T)*x.size());
        return *this;
    }

    void fsync() {
        int res = ::fsync(fd);
        assert(res != -1);
    }

    void close() {
        // write epilogue to ensure consistency between reads and writes
        write<int32_t>(0xBADABEE);
        int res = ::close(fd);
        assert(res != -1);
    }
};

class SafeFileWriter {
    std::string filename;
    FileWriter writer;
public:
    SafeFileWriter(const std::string& _filename)
        : filename(_filename), writer(_filename + ".tmp")  {}

    ~SafeFileWriter() {
        writer.fsync();
        writer.close();
        if (std::filesystem::exists(filename + ".old2"))
            std::filesystem::rename(filename + ".old2", filename + ".old3");
        if (std::filesystem::exists(filename + ".old"))
            std::filesystem::rename(filename + ".old", filename + ".old2");
        // copy instead of renaming so that replacing with new version is atomic
        if (std::filesystem::exists(filename))
            std::filesystem::copy(filename, filename + ".old");
        std::filesystem::rename(filename + ".tmp", filename);
    }

    template<typename T>
    FileWriter& write(const T& x) {
        return writer.write<T>(x);
    }

    template<typename T>
    FileWriter& write(const std::vector<T>& x) {
        return writer.write<T>(x);
    }
};

class FileReader {
    std::ifstream in;
public:
    FileReader(const std::string& filename) : in(filename, std::ios::binary) {
        assert(std::filesystem::exists(filename));
    }

    template<typename T>
    T read() {
        char buf[sizeof(T)];
        in.read(buf, sizeof(T));
        assert(!in.fail());
        assert(!in.eof());
        return *(T*)buf;
    }

    template<typename T>
    FileReader& read(std::vector<T>& x) {
        auto sz = read<size_t>();
        x.resize(sz);
        in.read((char*)&x[0], sizeof(T)*sz);
        assert(!in.fail());
        assert(!in.eof());
        return *this;
    }

    void Close() {
        // check epilogue
        int32_t epi = read<int32_t>();
        assert(epi == 0xBADABEE);
        in.close();
        assert(!in.fail());
    }
};
