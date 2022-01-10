
#include "src/tools/lru_cache.h"
#include "unit_test_frame/unittest.h"
#include <iostream>
#include <sstream>
#include <vector>


FIXTURE_BEGIN(LRUCacheTest)

CASE(TestLRUCacheWithoutLock){
    using KVCache = ToolBox::LRUCache<std::string, int32_t>;
    using KVNode = KVCache::node_type;
    auto CachePrint = [&](const KVCache& kvc){
        fprintf(stderr, "Cache(size:%zu)(max:%zu)(e=%zu)(allowed:%zu)\n", kvc.Size(), kvc.GetMaxSize(), kvc.GetElasticity(), kvc.GetMaxAllowedSize());
        std::size_t index = 0;
        const auto node_print = [&](const KVNode& kvn){
            fprintf(stderr, " ... [%zu] %s => %d\n", ++index, kvn.key.c_str(), kvn.value);
        };
        kvc.Foreach(node_print);
    };

    KVCache kvc(5,2);
    kvc.Insert("hello", 1);
    kvc.Insert("world", 2);
    kvc.Insert("foo", 3);
    kvc.Insert("bar", 4);
    kvc.Insert("github", 5);
    CachePrint(kvc);
    kvc.Insert("toolbox", 6);
    CachePrint(kvc);
    kvc.Insert("liyakai", 7);
    CachePrint(kvc);
    int32_t value_out;
    kvc.Get("github",value_out);
    fprintf(stderr, "github 应该移动到第一个.\n");
    CachePrint(kvc);
    kvc.Get("foo",value_out);
    fprintf(stderr, "world 应该移动到第一个.\n");
    CachePrint(kvc);
}

CASE(TestLRUCacheWithLock){
    using LKVCache = ToolBox::LRUCache<std::string, std::string, std::mutex>;
    using LKVNode = LKVCache::node_type;
    auto CachePrint = [&](const LKVCache& kvc){
        fprintf(stderr, "Cache(size:%zu)(max:%zu)(e=%zu)(allowed:%zu)\n", kvc.Size(), kvc.GetMaxSize(), kvc.GetElasticity(), kvc.GetMaxAllowedSize());
        std::size_t index = 0;
        const auto node_print = [&](const LKVNode& kvn){
            fprintf(stderr, " ... [%zu] %s => %s\n", ++index, kvn.key.c_str(), kvn.value.c_str());
        };
        kvc.Foreach(node_print);
    };
    LKVCache lkvc(25, 2);
    auto worker = [&](){
        std::ostringstream os;
        os << std::this_thread::get_id();
        std::string id = os.str();

        for(int32_t i = 0; i < 10; i++)
        {
            std::ostringstream os2;
            os2 << "id:" << id << ":" << i;
            lkvc.Insert(os2.str(), id);
        }
    };
    std::vector<std::unique_ptr<std::thread>> workers;
    workers.reserve(100);
    for(int32_t i = 0; i < 100; i++)
    {
        workers.push_back(std::unique_ptr<std::thread>(new std::thread(worker)));
    }

    for(const auto &worker : workers)
    {
        worker->join();
    }
    fprintf(stderr, " workers finished.\n");
    CachePrint(lkvc);
}

FIXTURE_END(AWLRUCacheTest)

