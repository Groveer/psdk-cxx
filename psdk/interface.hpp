#ifndef PSDK_INTERFACE_HPP
#define PSDK_INTERFACE_HPP

#include <string>

namespace psdk {
/** @brief Interface class that provides plugin's metadata and instantiate exported classes */
struct IPluginInfo
{
    virtual const std::string name() const = 0;
    virtual const std::string version() const = 0;

    virtual size_t classCount() const = 0;
    virtual const std::string className(size_t index) const = 0;

    /** Instantiate a class from its name */
    virtual void *instance(const std::string &className) const = 0;
};
}  // namespace psdk
#endif
