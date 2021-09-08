#pragma once

#include <string>
#include <map>


class SquidInternalAction
{
    public:
        static SquidInternalAction *getInstance();

        void add(std::string dbpath);
        std::map<std::string, int>::iterator begin();
        std::map<std::string, int>::iterator end();

    private:
        static SquidInternalAction *instance;

    protected:
        SquidInternalAction()
        { }

        std::map<std::string, int> hash;
};
