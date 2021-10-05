#include "src/tools/skip_list.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(SkipList)

struct Score
{
int32_t id = -1;
int32_t chinese = 0;
int32_t math = 0;
int32_t english = 0;
public: 
    Score()
        : id(-1),chinese(0),math(0),english(0)
    {}
    Score(int32_t i, int32_t c, int32_t m, int32_t e)
        :id(i), chinese(c), math(m), english(e)
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
        if(sumMe > sumHe)
        {
            return true;
        } else if(sumMe == sumHe)
        {
            if(this->chinese > d.chinese)
            {
                return true;
            } else if (this->chinese == d.chinese)
            {
                if(this->math > d.math)
                {
                    return true;
                } else if(this->math == d.math)
                {
                    if(this->english > d.english)
                    {
                        return true;
                    } else if(this->english == d.english)
                    {
                        return id > d.id;
                    } else 
                    {
                        return false;
                    }
                } else 
                {
                    return false;
                }
            } else 
            {
                return false;
            }
        } else 
        {
            return false;
        }
    }
    bool operator < (const Score& d)
    {
        if(*this == d)
        {
            return false;
        }
        return !(*this > d);
    }
};

CASE(SkipListCase1){
    RankSkipList<Score,int32_t>* rank_skip_list = new RankSkipList<Score, int32_t>(100);
    for(int32_t i = 15; i <= 100000; i++)
    {
        rank_skip_list->InsertOrUpdate(Score(i, std::rand() % 100 + 1, std::rand() % 100 + 1, std::rand() % 100 + 1), i);
    }

}


FIXTURE_END(SkipList)