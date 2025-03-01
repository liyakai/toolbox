#pragma once

#include <concepts>
#include "tools/singleton.h"


namespace ToolBox
{

class PluginInterface 
{
public:
    virtual ~PluginInterface()
    {
        PluginMgr->unregisterPlugin(this)
    }
    virtual void execute() = 0; // 执行插件
protected:
    PluginInterface() 
    {
        PluginMgr->registerPlugin(this);
    }
};

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

    void executeAllPlugins()
    {
        for (auto plugin : plugins)
        {
            plugin->execute();
        }
    }
private:
    std::vector<PluginInterface*> plugins;
};


/*
* 定义插件管理器单件
*/
#define PluginMgr Singleton<PluginManager>::Instance()

}   // namespace ToolBox