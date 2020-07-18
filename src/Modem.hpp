#ifndef MODEM_HPP
#define MODEM_HPP

#include "Common.hpp"

class Value;

bool modulate(Value& value,
              std::string& signal,
              std::string* pMsg = nullptr);

bool demodulate(const std::string& signal,
                Value& value,
                std::string* pMsg = nullptr);

#endif
