#pragma once

#include <numerals.h>

/* In libCat, the exit() function is provided globally instead of in <cstdlib>.
 * This streamlines out the existence of _exit(). */
extern "C" __attribute__((used)) void exit(i32 exit_code) {
    asm(R"(syscall)" : : "D"(decay_numeral(exit_code)), "a"(60));
    __builtin_unreachable();  // This elides a retq instruction.
}