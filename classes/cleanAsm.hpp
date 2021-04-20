#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstdlib>

#include "Static.hpp"

namespace ServiceFunc {
    char* ReadFile(const char* fname);
    void *ExpandData(uint32_t *capacity, uint32_t size, void *data, uint32_t el_size);
    void Copy(void *where_to, const void *where_from, uint32_t el_size);
    bool IsValidLine(const char *line);
    bool IsLabel(const char *lexem);
}

struct Label {
	uint32_t mark;
	char *label_name;
};

struct Labels {
	uint32_t capacity;
	uint32_t size;
	Label *labels;
    uint32_t FindLabel(const char *label) const;
    ~Labels();
    Labels();
    void PushLabel(char *label, uint32_t adrs);
	void LabelOutput();
};

struct BinaryCode {
	uint8_t *code;
	uint32_t capacity;
	uint32_t offset;
	uint32_t length;
	void BinaryOutput();
    ~BinaryCode();
    BinaryCode();

    void PushByAdrs(uint32_t adrs, void *flag, uint32_t el_size);
    void PushInBin(const void *elem, uint32_t el_size);
	void WriteBin();
};	

struct AsmLine {
	//each line holds array of string lexems.
	char **lexems;
	uint32_t nlexems;
    AsmLine();
    AsmLine(char *line, uint32_t numlexems);
    void Fill(char *line, uint32_t numlexems);
    ~AsmLine();
    private:
    //divides line into lexems
	void SetLineLexems(char *line);
};

struct AsmLineList {
	//list of lines
	AsmLine *lines = nullptr;
    char **ptr_list = nullptr;
	//general number of lines
	uint32_t nlines = 0;
    ~AsmLineList();
    AsmLineList();
    void Parse(char *buff);
	void LexemsOutput() const;
    uint32_t CountLexems(const char *line) const;
    void CommentsProcessing();
    void CountLines(char *buff);
    //gets list of non-empty lines
	char **GetLines(char *buff) const;
};

namespace ServiceFunc {
    char* ReadFile(const char* fname);
    void *ExpandData(uint32_t *capacity, uint32_t size, void *data, uint32_t el_size);
    void Copy(void *where_to, const void *where_from, uint32_t el_size);
    //check if line contains valid lexems(not dividers and comments))
    bool IsValidLine(const char *line);
    bool IsLabel(const char *lexem);
    bool IsCtrlFlow(uint32_t ctrl_flow);
    bool IsRegister(char *argument);
}

class ASM {
	private:
	static AsmLineList list;
	//code data
	static BinaryCode cdata;
	//list of labels.
	static Labels llabels;   														
    static void ArgumentProcess(uint32_t arg, uint32_t *pos, uint8_t *flag, uint32_t ctrl_flow, uint32_t pass, uint32_t curline);
    //processing functions that change Flow Order and have >=1 argument
    static void CtrlFlowProcess(char *label, uint32_t pass);
    //[...] argument processing
    static uint32_t RamProcess(char *lexem, uint8_t *flag);
    static uint32_t DefaultProcess(char *lexem, uint8_t *flag);
    static uint32_t ProcessAdditional(char *lexem, uint8_t *flag, const char *endline);
    public:
	static void PrintBinary();
    static void PrintList();
	static void PrintLabels();
	static void WriteInBin();
    static void TranslateBin();
    static void MainProcess();
    static void ParseShell(char *buff);
};