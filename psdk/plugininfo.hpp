#ifndef PSDK_PLUGININFO_HPP
#define PSDK_PLUGININFO_HPP

#include <functional>
#include <string>
#include <vector>

#include "interface.hpp"

/** Macro EXPORT_CPP makes a symbol visible. */
#if defined(_WIN32)
// MS-Windows NT
#define PSDK_EXPORT_C extern "C" __declspec(dllexport)
#else
// Unix-like OSes
#define PSDK_EXPORT_C extern "C" __attribute__((visibility("default")))
#endif

namespace psdk {

using std::string;
/** @brief Concrete implementation of the interface IPluginFactory.
 * An instance of this class is supposed to be exported by the plugin
 * entry-point function
 */
class PluginInfo : public IPluginInfo
{
    using Item = std::pair<string, std::function<void *()>>;
    using Items = std::vector<Item>;

    string m_name;
    string m_version;
    Items m_items;

public:
    PluginInfo(const string &name, const string &version)
        : m_name(name)
        , m_version(version)
    {
    }

    const string name() const override { return m_name; }
    void setName(const string &name) { m_name = name; }

    const string version() const override { return m_version; }
    void setVersion(const string &version) { m_version = version; }

    virtual size_t classCount() const override { return m_items.size(); }
    virtual const string className(size_t index) const override { return m_items[index].first; }

    /** Instantiate a class from its name.
     * This member function returns a type-erased pointer
     * to a class object allocated on the heap.
     */
    void *instance(const string &className) const override
    {
        auto it = std::find_if(m_items.begin(), m_items.end(), [&className](const auto &p) { return p.first == className; });
        if (it == m_items.end())
            return nullptr;
        return it->second();
    }

    /** Register class name and constructor in the plugin database */
    template <typename AClass>
    PluginInfo &registerClass(const string &name)
    {
        auto constructor = [] { return new (std::nothrow) AClass; };
        m_items.push_back(std::make_pair(name, constructor));
        return *this;
    }
};
}  // namespace psdk

#endif
