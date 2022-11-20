#ifndef PSDK_LOADER_HPP
#define PSDK_LOADER_HPP

#include <limits.h>

#include <cassert>
#include <map>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#include "interface.hpp"

// Unix
#if defined(_WIN32)
#include <windows.h>
#elif defined(__unix__)
// APIs: dlopen, dlclose, dlopen
#include <dlfcn.h>
#else
#error "Not supported operating system"
#endif

namespace psdk {

using std::string;

/** @brief Class form managing and encapsulating shared libraries loading  */
class Plugin
{
public:
    /** @brief Function pointer to DLL entry-point */
    using GetPluginInfo_fp = IPluginInfo *(*)();
    /** @brief Name of DLL entry point that a Plugin should export */
    static constexpr const char *DLLEntryPointName = "PluginInfo";

    /** @brief Shared library handle */
    void *m_hnd = nullptr;
    /** @brief Shared library file name */
    std::string m_file = "";
    /** @brief Flag to indicate whether plugin (shared library) is loaded into current process. */
    bool m_isLoaded = false;
    /** @brief Pointer to shared library factory class returned by the DLL entry-point function */
    IPluginInfo *m_info = nullptr;

    Plugin() {}

    explicit Plugin(const string &file)
    {
        m_file = file;
        char real_path[PATH_MAX];
        realpath(m_file.c_str(), real_path);
#if !defined(_WIN32)
        m_hnd = ::dlopen(real_path, RTLD_LAZY);
#else
        m_hnd = (void *)::LoadLibraryA(real_path);
#endif
        if (m_hnd == nullptr) {
            spdlog::error("Cannot open library: {}", real_path);
            return;
        }
        m_isLoaded = true;
#if !defined(_WIN32)
        auto dllEntryPoint = reinterpret_cast<GetPluginInfo_fp>(dlsym(m_hnd, DLLEntryPointName));
#else
        auto dllEntryPoint = reinterpret_cast<GetPluginInfo_fp>(GetProcAddress((HMODULE)m_hnd, DLLEntryPointName));
#endif
        if (dllEntryPoint == nullptr)
            spdlog::warn("Cannot get plugin info: {}", real_path);
        // Retrieve plugin metadata from DLL entry-point function
        m_info = dllEntryPoint();
    }

    ~Plugin() { this->unload(); }
    Plugin(const Plugin &) = delete;
    Plugin &operator=(const Plugin &) = delete;

    Plugin(Plugin &&rhs)
    {
        m_isLoaded = std::move(rhs.m_isLoaded);
        m_hnd = std::move(rhs.m_hnd);
        m_file = std::move(rhs.m_file);
        m_info = std::move(rhs.m_info);
    }
    Plugin &operator=(Plugin &&rhs)
    {
        std::swap(rhs.m_isLoaded, m_isLoaded);
        std::swap(rhs.m_hnd, m_hnd);
        std::swap(rhs.m_file, m_file);
        std::swap(rhs.m_info, m_info);
        return *this;
    }

    IPluginInfo *pluginInfo() const { return m_info; }

    void *createInstance(const string &className) { return m_info->instance(className); }

    bool isLoaded() const { return m_isLoaded; }

    void unload()
    {
        if (m_hnd != nullptr) {
#if !defined(_WIN32)
            ::dlclose(m_hnd);
#else
            ::FreeLibrary((HMODULE)m_hnd);
#endif
            m_hnd = nullptr;
            m_isLoaded = false;
        }
    }
};

/** @brief Repository of plugins.
 * It can instantiate any class from any loaded plugin by its name.
 **/
class PluginManager
{
public:
    using PluginMap = std::map<std::string, Plugin>;
    // Plugins database
    PluginMap m_plugindb;

    PluginManager() {}

    IPluginInfo *addPlugin(const std::string &name)
    {
        auto &&plugin = Plugin(name);
        if (plugin.pluginInfo() == nullptr)
            return nullptr;
        auto &&infoName = plugin.pluginInfo()->name();
        m_plugindb[infoName] = std::move(plugin);
        return m_plugindb[infoName].pluginInfo();
    }

    IPluginInfo *pluginInfo(const char *pluginName)
    {
        auto it = m_plugindb.find(pluginName);
        if (it == m_plugindb.end())
            return nullptr;
        return it->second.pluginInfo();
    }

    /** @brief Instantiate a class exported by some loaded plugin.
     *  @tparam T           Interface (interface class) implemented by the loaded class.
     *  @param  pluginName  Name of the plugin that exports the class.
     *  @param  className   Name of the class exported by the plugin.
     *  @return             Instance of exported class casted to some interface T.
     * */
    template <typename T>
    std::shared_ptr<T> createInstance(std::string pluginName, std::string className)
    {
        auto &&it = m_plugindb.find(pluginName);
        if (it == m_plugindb.end())
            return nullptr;
        void *pObj = it->second.createInstance(className);
        return std::shared_ptr<T>(reinterpret_cast<T *>(pObj));
    }

}; /* --- End of class PluginManager --- */

}  // namespace psdk

#endif
