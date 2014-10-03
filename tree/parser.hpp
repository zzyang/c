#ifndef __PARSER__
#define __PARSER__


#include "ast.hpp"
#include <string>

bool doParse(const std::string& input, ast::regex& data);

#endif // __PARSER__
