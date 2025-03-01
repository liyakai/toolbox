#pragma once

#include <concepts>
#include "tools/singleton.h"


namespace ToolBox
{

template<typename T>
concept PluginConcept = requires(T t) {
    {t.execute()} -> std::same_as<void>;
};

class PluginInterface 
{
public:
    virtual ~PluginInterface() = default;
    virtual void execute() = 0; // 执行插件
};

class PluginManager
{
public:
    template<typename T>
    void registerPlugin(T* plugin)
    {
        if constexpr (PluginConcept<T>)
        {
            plugins.push_back(plugin);
        }
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