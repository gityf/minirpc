/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CStackTrace.
*/


#include <ctype.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include "common/debug/cstack_trace.h"
namespace wyf {

const char kMangledPrefix[] = "_Z";

std::string GetDemangleSymbol(char* mangled_symbol)
{
    std::string demangled;

    char *p = mangled_symbol;
    while (*p != 0) {
        char* begin = strstr(p, kMangledPrefix);
        // if no demangle prefix found, then finished.
        if (begin == NULL) {
            demangled.append(p);
            break;
        }
        // append these characters before the prefix unchanged
        demangled.append(p, begin);
        char* end = begin;
        while (*end != 0 && (isalnum(*end) || (*end == '_'))) {
            ++end;
        }
        char origin = *end;
        // abi::__cxa_demangle requires a string ended with \0
        *end = 0;
        int status = 0;
        char* demangled_symbol = abi::__cxa_demangle(begin,
                                                     NULL,
                                                     0,
                                                     &status);
        *end = origin;
        if (demangled_symbol) {
            demangled.append(demangled_symbol);
            free(demangled_symbol);
        } else {
            // if failes, append the "_Z" continue to search
            demangled.append(begin, sizeof(kMangledPrefix) - 1);
            end = begin + sizeof(kMangledPrefix) - 1;
        }
        p = end;
    }
    return demangled;
}

void DumpCStackTraceToStrings(void* const* stacktrace,
                             int count,
                             std::vector<std::string>* str)
{
    char** mangled_symbols = backtrace_symbols(stacktrace, count);
    if (mangled_symbols == 0) {
        return;
    }
    std::vector<std::string> output;
    for (int k = 0; k < count; ++k) {
        std::string symbol = GetDemangleSymbol(mangled_symbols[k]);
        output.push_back(symbol);
    }
    str->swap(output);
    free(mangled_symbols);
}

CStackTrace::CStackTrace() {
    int count = backtrace(mTrace, kMaxCallStack);
    mCount = std::max(count, 0);
}

CStackTrace::~CStackTrace() {
}

int CStackTrace::ToString(std::string* stack) const
{
    std::stringstream ss;
    int size = Output(&ss);
    *stack = ss.str();
    return size;
}

int CStackTrace::Output(std::ostream* os) const
{
    std::vector<std::string> stacks;
    DumpCStackTraceToStrings(mTrace, mCount, &stacks);
    size_t count = stacks.size();
    int size = 0;
    if (count <= 1) {
        *os << "<No stack traces>\n";
    } else {
        for (size_t k = 1; k < count; ++k) {
            *os << "\t" << stacks[k] << "\n";
        }
        size = static_cast<int>(count - 1);
    }
    return size;
}

} // namespace wyf
