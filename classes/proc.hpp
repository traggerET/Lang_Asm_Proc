#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstdlib>

const double EPS = 1e-6;
//const int DEFSZ = 1000;

#include "Static.hpp"

struct BinCmd {
    uint8_t* bcode;
    uint32_t length;
    uint32_t offset;
    BinCmd(const char *fname);
    ~BinCmd();
    void BinOutput();
};

template <typename T>
class Stack {
    private:
    uint32_t size;
    uint32_t capacity;
    T *stack;
    public:
    Stack();
    ~Stack();
    T PeekTop() const;
    T Pop();
    void Push(T val);
    void StackDump();
};

class Processor {
    static BinCmd cdata;
    static Stack<double> stk;
    static Stack<uint32_t> stkcall;
    static double registers[4];
    static double* ram;
    //file name where your program will write necessary results
    static const char *output;
    public:
    static void SetOutput(const char *fname);
    static void StackOut();
    static void Execute();
    static void BinPrint();
    static void MainProcess();
};