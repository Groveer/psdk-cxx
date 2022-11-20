#include <cassert>
#include <iostream>
#include "psdk/loader.hpp"

class IMathFunction
{
public:
    virtual const char *Name() const = 0;
    virtual double Eval(double x) const = 0;
    virtual ~IMathFunction() = default;
};

using psdk::IPluginInfo;
using psdk::PluginManager;

int main()
{
    PluginManager ma;
    IPluginInfo *pluginInfo = ma.addPlugin("libplugin.so");
    // * infoA = ma.GetPluginInfo("PluginA");
    assert(pluginInfo != nullptr);

    std::cout << " ---- Plugin Information --------- "
              << "\n"
              << "  =>              Name = " << pluginInfo->name() << "\n"
              << "  =>           Version = " << pluginInfo->version() << "\n"
              << "  => Number of classes = " << pluginInfo->classCount() << "\n\n";

    std::cout << "Classes exported by the Plugin: "
              << "\n";

    for (size_t n = 0; n < pluginInfo->classCount(); n++) {
        auto instance = ma.createInstance<IMathFunction>(pluginInfo->name(), pluginInfo->className(n));
        if (instance != nullptr) {
            std::cout << "Name: " << instance->Name() << "\n";
            std::cout << "Eval: " << instance->Eval(3.0) << "\n";
        }
    }

    return 0;
}
