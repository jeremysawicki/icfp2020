#ifndef EVAL_HPP
#define EVAL_HPP

#include "Common.hpp"

class Value;

bool eval(Value& value,
          std::string* pMsg = nullptr);

#endif
