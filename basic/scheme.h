#pragma once

#include <string>
#define SCHEME_FUZZING_2_PRINT_REQUESTS

class Interpreter {
public:
    std::string Run(const std::string& s);
};
