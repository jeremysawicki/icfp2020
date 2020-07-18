#ifndef SYMTABLE_HPP
#define SYMTABLE_HPP

#include "Common.hpp"
#include <unordered_map>

class Expr;

class SymTable
{
public:
    bool getName(uint32_t id, std::string* pName) const
    {
        if (id < m_idToName.size())
        {
            if (pName) *pName = m_idToName[id];
            return true;
        }

        return false;
    }

    bool getId(const std::string& name, uint32_t* pId) const
    {
        auto findIt = m_nameToId.find(name);
        if (findIt != m_nameToId.end())
        {
            if (pId) *pId = findIt->second;
            return true;
        }

        return false;
    }

    uint32_t getOrAdd(const std::string& name)
    {
        auto findIt = m_nameToId.find(name);
        if (findIt != m_nameToId.end())
        {
            return findIt->second;
        }
        else
        {
            uint32_t id = m_idToName.size();
            m_idToName.push_back(name);
            m_nameToId.emplace(name, id);
            return id;
        }
    }

    uint32_t size() const { return m_idToName.size(); }

private:
    std::vector<std::string> m_idToName;
    std::unordered_map<std::string, uint32_t> m_nameToId;
};

#endif
