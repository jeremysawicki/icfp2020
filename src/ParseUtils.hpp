#ifndef PARSEUTILS_HPP
#define PARSEUTILS_HPP

#include "Common.hpp"

bool parseU32(const std::string& str, uint32_t* pValue = nullptr);
bool parseI32(const std::string& str, int32_t* pValue = nullptr);
bool parseU64(const std::string& str, uint64_t* pValue = nullptr);
bool parseI64(const std::string& str, int64_t* pValue = nullptr);
bool parseFloat(const std::string& str, float* pValue = nullptr);
bool parseDouble(const std::string& str, double* pValue = nullptr);

#endif
