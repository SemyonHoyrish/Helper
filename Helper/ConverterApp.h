#pragma once
#include <ostream>
#include <deque>
#include <map>
#include <algorithm>
#include <set>
#include <functional>
#include <string>
#include <assert.h>
#include "App.h"
#include "MultiKeyMap.h"
#include "Value.h"
#include "ArgParser.h"
#include "CommonLib.h"


// TODO: replace conversion with a conversion graph.

using std::map;
using std::set;
using std::vector;
using std::pair;

template <typename NodeT, typename EdgeT>
class ConversionGraph
{
public:

    void insertNode(const NodeT* n);
    void connect(const NodeT* a, const NodeT* b, const EdgeT edge);

    void removeNode(const NodeT* n) = delete;


    vector<const EdgeT*> searchPath(const NodeT* a, const NodeT* b);


private:


private:
    map<const NodeT*, vector<pair<const NodeT*, const EdgeT>>> edges;

};


struct Unit {
    Unit(std::string sn, std::string n) : shortname(sn), name(n) {}

    const std::string shortname;
    const std::string name;
};

//struct ConvVal {
//    enum class Tag { INVALID, INTEGER, FLOAT, STRING } tag;
//    union {
//        int64_t ival;
//        long double fval;
//        const char* sval;
//    };
//};

struct Conversion {
    Unit from;
    Unit to;

    //std::function<std::vector<ConvVal>(std::vector<ConvVal>)> f;
    //std::function<ConvVal(ConvVal)> f;
    std::function<Value(Value)> f;
};


class ConverterApp :
    public App
{
public:
    static const std::string Name;

    explicit ConverterApp() : App() {
    
#ifdef _DEBUG
        assert(_validate());
#endif

        init();
    }

    //COMMONLIB_API Status run(std::vector<std::string> args) override;
    Status run(const char* const args[], int count) override;

    static std::ostream& stream;

private:

    void init();

    bool _validate() {
        // TODO:
        return true;
    }

private:
    static const MultiKeyMap<std::string, Conversion> convs;

    ConversionGraph<Unit, std::function<Value(Value)>> conversions;

};
