#include "tools/skip_list.h"
#include "unit_test_frame/unittest.h"
#include <map>
#include <inttypes.h>
#include <stdint.h>

FIXTURE_BEGIN(SkipList)

struct Score
{
    int32_t id = -1;
    int32_t chinese = 0;
    int32_t math = 0;
    int32_t english = 0;
public:
    Score()
        : id(-1), chinese(0), math(0), english(0)
    {}
    Score(int32_t i, int32_t c, int32_t m, int32_t e)
        : id(i), chinese(c), math(m), english(e)
    {
    }
    int32_t Sum()
    {
        return chinese + math + english;
    }
    bool operator == (const Score& d)
    {
        return this->id == d.id && this->chinese == d.chinese &&
               this->math == d.math && this->english == d.english;
    }
    bool operator != (const Score& d)
    {
        return !(*this == d);
    }
    bool operator > (const Score& d)
    {
        int32_t sumMe = chinese + math + english;
        int32_t sumHe = d.chinese + d.math + d.english;
        if (sumMe > sumHe)
        {
            return true;
        }
        else if (sumMe == sumHe)
        {
            if (this->chinese > d.chinese)
            {
                return true;
            }
            else if (this->chinese == d.chinese)
            {
                if (this->math > d.math)
                {
                    return true;
                }
                else if (this->math == d.math)
                {
                    if (this->english > d.english)
                    {
                        return true;
                    }
                    else if (this->english == d.english)
                    {
                        return id > d.id;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    bool operator < (const Score& d)
    {
        if (*this == d)
        {
            return false;
        }
        return !(*this > d);
    }
};

// 测试数据插入、删除、获取排名
CASE(SkipListCase)
{
    // 创建一个长度为100的排行榜跳表
    ToolBox::RankSkipList<Score, int32_t>* rank_skip_list = new ToolBox::RankSkipList<Score, int32_t>(10);
    // 随机插入十万条数据
    for (int32_t i = 15; i <= 100000; i++)
    {
        rank_skip_list->InsertOrUpdate(Score(i, std::rand() % 100 + 1, std::rand() % 100 + 1, std::rand() % 100 + 1), i);
    }
    // 手工插入几条数据
    rank_skip_list->InsertOrUpdate(Score(1, 100, 99, 100), 1);
    rank_skip_list->InsertOrUpdate(Score(2, 99, 99, 99), 2);
    rank_skip_list->InsertOrUpdate(Score(3, 99, 99, 99), 3);
    rank_skip_list->InsertOrUpdate(Score(101, 100, 100, 100), 101);
    std::map<int32_t, bool> res;
    // 删除部分数据
    for (int32_t i = 1; i <= 200; i++)
    {
        int32_t ele = 20 + i * 400;
        rank_skip_list->DeleteNode(ele);
        res[ele] = true;
    }
    // 打印排行榜
    fprintf(stderr, "[跳表排行榜] 元素数量:%" PRIu64" \n", rank_skip_list->Length());
    int32_t i = 1;
    ToolBox::SkipListNode<Score, int32_t>* node = rank_skip_list->GetNodeByRank(i);
    do
    {
        if (nullptr != node)
        {
            Score score = node->key;
            // 排行榜不可能重复
            if (res.find(score.id) != res.end())
            {
                fprintf(stderr, "[跳表排行榜] error, id:%d\n", score.id);
                SetError("[跳表排行榜] 出现重复\n");
                return;
            }
            fprintf(stderr, "[跳表排行榜] rank=% " PRId64", id=%d, sum=%d, chinese=%d, math=%d, english=%d\n",
                    rank_skip_list->Rank(score.id), score.id, score.Sum(), score.chinese, score.math, score.english);
            res[score.id] = true;
            i++;
            node = rank_skip_list->GetNodeByRank(i);
        }
    }
    while (nullptr != node);
    fprintf(stderr, "==============================\n");
    fprintf(stderr, "测试删除\n");
    // 删除第一名5次
    for (int32_t i = 1; i <= 5; i++)
    {
        rank_skip_list->DeleteNodeByRank(1);
    }
    rank_skip_list->DeleteNodeByRange(5, 10);
    i = 1;
    node = rank_skip_list->GetNodeByRank(i);
    do
    {
        if (nullptr != node)
        {
            Score score = node->key;
            fprintf(stderr, "[跳表排行榜] rank=% " PRId64", id=%d, sum=%d, chinese=%d, math=%d, english=%d\n",
                    rank_skip_list->Rank(score.id), score.id, score.Sum(), score.chinese, score.math, score.english);
            res[score.id] = true;
            i++;
            node = rank_skip_list->GetNodeByRank(i);
        }
    }
    while (nullptr != node);
    fprintf(stderr, "==============================\n");
    fprintf(stderr, "测试获取范围内的数据  \n");
    auto list = rank_skip_list->GetNodesByRange(2, 8);
    for (auto it : list)
    {
        Score score = it -> key;
        fprintf(stderr, "[跳表排行榜] rank=% " PRId64" id=%d, sum=%d, chinese=%d, math=%d, english=%d\n",
                rank_skip_list->Rank(score.id), score.id, score.Sum(), score.chinese, score.math, score.english);
        res[score.id] = true;
    }
    return;
}
// 测试其他零碎的功能
CASE(SkipListCase2)
{
    // 创建一个长度为100的排行榜跳表
    ToolBox::RankSkipList<Score, int32_t>* rank_skip_list = new ToolBox::RankSkipList<Score, int32_t>(10);
    // 随机插入十万条数据
    for (int32_t i = 15; i <= 100000; i++)
    {
        rank_skip_list->InsertOrUpdate(Score(i, std::rand() % 100 + 1, std::rand() % 100 + 1, std::rand() % 100 + 1), i);
    }
    rank_skip_list->InsertOrUpdate(Score(1, 100, 99, 100), 1);
    if (!rank_skip_list->IsAlreadyexists(1))
    {
        SetError("[跳表排行榜] IsAlreadyexists 功能异常.\n");
    }
    if (rank_skip_list->IsAlreadyexists(2))
    {
        SetError("[跳表排行榜] IsAlreadyexists 功能异常.\n");
    }
    auto* score  = rank_skip_list->Score(1);
    if (nullptr == score)
    {
        SetError("[跳表排行榜] Score 功能异常.\n");
    }
    else
    {
        fprintf(stderr, "[跳表排行榜] 通过 value 找到 key rank=% " PRId64" id=%d, sum=%d, chinese=%d, math=%d, english=%d\n",
                rank_skip_list->Rank(score->id), score->id, score->Sum(), score->chinese, score->math, score->english);
    }

    // 打印排行榜
    int32_t i = 1;
    ToolBox::SkipListNode<Score, int32_t>* node = rank_skip_list->GetNodeByRank(i);
    do
    {
        if (nullptr != node)
        {
            Score score = node->key;
            fprintf(stderr, "[跳表排行榜] rank=% " PRId64", id=%d, sum=%d, chinese=%d, math=%d, english=%d \n",
                    rank_skip_list->Rank(score.id), score.id, score.Sum(), score.chinese, score.math, score.english);
            i++;
            node = rank_skip_list->GetNodeByRank(i);
        }
    }
    while (nullptr != node);
}
FIXTURE_END(SkipList)