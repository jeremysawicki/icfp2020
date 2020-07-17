#ifndef CODE_HPP
#define CODE_HPP

#include "Common.hpp"
#include "Inst.hpp"

class Entry
{
public:
    uint32_t m_iInst;
    uint32_t m_iSlot;
};

class Code
{
public:
    std::vector<Inst> m_insts;
    std::vector<Entry> m_entries;
    std::vector<uint32_t> m_slotMap;
};

#endif
