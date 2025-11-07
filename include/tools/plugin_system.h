#pragma once

#include <vector>
#include <algorithm>
#include "tools/singleton.h"


namespace ToolBox
{
class PluginInterface;

class PluginManager
{
public:
    void registerPlugin(PluginInterface* plugin)
    {
        plugins.emplace_back(plugin);
    }
    void unregisterPlugin(PluginInterface* plugin)
    {
        plugins.erase(std::remove(plugins.begin(), plugins.end(), plugin), plugins.end());
    }

    void updateAllPlugins();
private:
    std::vector<PluginInterface*> plugins;
};

#define PluginMgr Singleton<PluginManager>::Instance()

class PluginInterface 
{
public:
    virtual ~PluginInterface()
    {
        PluginMgr->unregisterPlugin(this);
    }
    virtual void pluginUpdate() = 0; // 执行插件
protected:
    PluginInterface() 
    {
        PluginMgr->registerPlugin(this);
    }
};

inline void PluginManager::updateAllPlugins()
{
    for (auto plugin : plugins)
    {
        plugin->pluginUpdate();
    }
}




}   // namespace ToolBox