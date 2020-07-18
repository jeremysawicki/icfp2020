#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "Common.hpp"

enum class Function : uint32_t
{
    Invalid,
    Inc, // #5
    Dec, // #6
    Add, // #7
    Mul, // #9
    Div, // #10
    Eq, // #11
    Lt, // #12
    Modulate, // #13
    Demodulate, // #14
    Send, // #15
    Neg, // #16
    S, // #18
    C, // #19
    B, // #20
    True, // #21
    False, // #22
    I, // #24
    Cons, // #25
    Car, // #26
    Cdr, // #27
    Nil, // #28
    IsNil, // #29
    Vec, // #31
    Draw, // #32
    Checkerboard, // #33
    MultipleDraw, // #34
    If0 // #37
};

#endif
