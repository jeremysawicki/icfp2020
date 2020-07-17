#ifndef TOKENTEXT_HPP
#define TOKENTEXT_HPP

#include "Common.hpp"

class Token;

bool parseTokenText(const std::string& text,
                    std::vector<Token>* pTokens,
                    std::string* pMsg = nullptr);

bool formatTokenText(const std::vector<Token>& tokens,
                     std::string* pText,
                     std::string* pMsg = nullptr);

#endif
