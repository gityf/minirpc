#include <iostream>
#include "common/debug/cstack_trace.h"


int main() {
    wyf::StackTrace* st = new wyf::StackTrace();
    std::string ststr;
    st->ToString(&ststr);
    std::cout << ststr << std::endl;
    delete st;
    return 0;
}