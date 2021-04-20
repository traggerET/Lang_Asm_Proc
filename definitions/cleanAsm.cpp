#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstdlib>

#include "../classes/cleanAsm.hpp"

//fills list with commands from your file.
AsmLineList ASM::list;
Labels ASM::llabels;
BinaryCode ASM::cdata;

uint32_t Labels::FindLabel(const char *label) const {
	for (auto i = 0; i < size; i++) {
		if (strcmp(labels[i].label_name, label) == 0)
			return labels[i].mark;
	}
	return -1;
}
Labels::~Labels() {
	for (auto i = 0; i < capacity; i++)
		free(labels[i].label_name);
	free(labels);
}
Labels::Labels() {
	capacity = DEFSIZE;
	size = 0;
	labels = (Label *)calloc(DEFSIZE, sizeof(Label));
}
void Labels::PushLabel(char *label, uint32_t adrs) {
	//printf("pushing label %s\n", label);
	labels = (Label *)ServiceFunc::ExpandData(&capacity, size, (void *)labels, sizeof(Label));
	uint32_t tmp_len = strlen(label);
	labels[size].label_name = (char *)calloc(tmp_len, sizeof(char));
	strncpy(labels[size].label_name, label, tmp_len - 1);
	labels[size++].mark = adrs;
}

void Labels::LabelOutput() {
	for (auto i = 0; i < size; i++)
		printf("!%s, %d1\n", labels[i].label_name, labels[i].mark);
}

void BinaryCode::BinaryOutput() {
	for (auto i = 0; i < length; i++)
		printf("%d ", code[i]);
}
BinaryCode::~BinaryCode() {
	free(code);
}
BinaryCode::BinaryCode() {
	capacity = DEFSIZE;
	code = (uint8_t *)calloc(capacity, sizeof(uint8_t)); 
	length = 0;
	offset = 0;
}

void BinaryCode::PushByAdrs(uint32_t adrs, void *flag, uint32_t el_size) {
	ServiceFunc::Copy(code + adrs, flag, el_size);
}
void BinaryCode::PushInBin(const void *elem, uint32_t el_size) {
	//if new size (length + el_size) is bigger than capacity, realloc.
	code = (uint8_t *)ServiceFunc::ExpandData(&capacity, length + el_size, code, el_size);
	assert(offset <= length);
	if (length >= offset + el_size)
		ServiceFunc::Copy(code + offset, elem, el_size);
	else {
		length += offset + el_size - length;
		ServiceFunc::Copy(code + offset, elem, el_size);
	}
	offset += el_size;
}

void BinaryCode::WriteBin() {
	FILE* fbin = fopen("binary.bin", "wb");
	assert(fbin);
	fwrite(code, sizeof(uint8_t), length, fbin);
	fclose(fbin);
}
AsmLine::~AsmLine() {
    // fprintf(stderr, "deleing asmline\n");
    delete[] lexems;
}
AsmLine::AsmLine() {};
AsmLine::AsmLine(char *line, uint32_t numlexems) {
	nlexems = numlexems;
	lexems = new char*[nlexems];
	SetLineLexems(line);
}
void AsmLine::Fill(char *line, uint32_t numlexems) {
	nlexems = numlexems;
	lexems = new char*[nlexems];
	SetLineLexems(line);
}
/*AsmLine::~AsmLine() {
	delete[] lexems;
}*/
    //divides line into lexems
void AsmLine::SetLineLexems(char *line) {
	if (line == nullptr) return;
	char *lexem;
	if ((lexem = strtok(line, " \t\0")) == nullptr) return;
	uint32_t i = 0;
	lexems[i++] = lexem;
	for (i = 1; i < nlexems; i++) {
		lexem = strtok(nullptr, " \t\0");
		//printf("lexem: %s\n", lexem);
		lexems[i] = lexem;
		//printf("lexems[%d]: %s\n", numl, lexems[numl]);
	}
}

AsmLineList::~AsmLineList() {
	printf("deleting asmlinelist\n");
	free(ptr_list);
	delete[] lines;
}
void AsmLineList::Parse(char *buff) {
	CountLines(buff);
	//get list of lines.
	ptr_list = GetLines(buff);
	lines = new AsmLine[nlines];
	//initialize each AsmLine: set lexem numbers, divide line into lexems and push them in the table of lexems.
	for (auto i = 0; i < nlines; i++) {
		lines[i].Fill(ptr_list[i], CountLexems(ptr_list[i]));
	}
	CommentsProcessing();
}
AsmLineList::AsmLineList() {}

void AsmLineList::LexemsOutput() const {
	for (auto i = 0; i < nlines; i++) {
		for (auto j = 0; j < lines[i].nlexems; j++) {
			printf("%s ", lines[i].lexems[j]);
		}
		printf("\n");
	}
}
uint32_t AsmLineList::CountLexems(const char *line) const {
	uint32_t nlexems = 0;
	char *tab_pos;
	char *space_pos;
	while (isspace(*line)) line++;
	tab_pos = (char *)strchr(line, '\t');
	space_pos = (char *)strchr(line, ' ');
	while (space_pos != nullptr || tab_pos != nullptr) {
		if ((space_pos != nullptr) && ((tab_pos == nullptr) || (space_pos < tab_pos))) line = space_pos;
		else 												 						   line = tab_pos;
		nlexems++;
		while (isspace(*line)) line++;
		tab_pos = (char *)strchr(line, '\t');
		space_pos = (char *)strchr(line, ' ');
	}
	//if last space has been faoun before valid symbol, add lexem.
	if (*line != '\0') nlexems++; 
	return nlexems;
}

void AsmLineList::CommentsProcessing() {
	for (auto i = 0; i < nlines; i++) {
		for (auto j = 0; j < lines[i].nlexems; j++) {
			char *comment_mark;
			if ((comment_mark = strchr(lines[i].lexems[j], ';')) != nullptr) {
				//if ';' is in the begin of the lexem, amount of lexemes is kept
				if (comment_mark == lines[i].lexems[j]) lines[i].nlexems = j;
				else 									lines[i].nlexems = j + 1;
				*comment_mark = '\0';
			}
		}
	}
}

void AsmLineList::CountLines(char *buff) {
	nlines = 0;
	if (buff == nullptr) return;
	char *tmp = buff;
	char *begin = tmp;
	while (*tmp != '\0') {
		if (*tmp != '\n') tmp++;
		else {
			*(tmp++) = '\0';
			if (ServiceFunc::IsValidLine(begin)) nlines++;
			begin = tmp;
		}
	}
	//if last line is not empty ("PUSH 9'\0'", for example)
	if (ServiceFunc::IsValidLine(begin)) nlines++;
}

//gets list of non-empty lines
char **AsmLineList::GetLines(char *buff) const {
	if (buff == nullptr) return nullptr;
	char *tmp = buff;
	char **lines = (char **)calloc(nlines, sizeof(char *));
	for (auto i = 0; i < nlines; i++) {
		//if not empty line
		//printf("%s\n", tmp);
		while (!ServiceFunc::IsValidLine(tmp)) 
			while (*(tmp++) != '\0');
		if (ServiceFunc::IsValidLine(tmp)) {
			//printf("!%s %d!", tmp, i);
			lines[i] = tmp;
			while (*(tmp++) != '\0');
		}
	}
	return lines;
}

/*char* ServiceFunc::ReadFile(const char* fname) {
	FILE* fin = fopen(fname, "r");
	assert(fin);
	fseek(fin, 0L, SEEK_END);
	//+1 for \0
	size_t size = ftell(fin) + 1; 
	fseek(fin, 0L, SEEK_SET);
	char* buff = (char*)calloc(size, sizeof(char));
	fread(buff, 1, size, fin);
	fclose(fin);
	return buff;
}*/
    
void *ServiceFunc::ExpandData(uint32_t *capacity, uint32_t size, void *data, uint32_t el_size) {
	void *new_data = data;
	if (size >= *capacity) {
		while (size >= *capacity)
			*capacity *= 2;
		new_data = (void *)realloc(data, el_size * (*capacity));
		assert(new_data != nullptr);
	}
	return new_data;
}

void ServiceFunc::Copy(void *where_to, const void *where_from, uint32_t el_size) {
	for (auto i = 0; i < el_size; i++) 
		*((uint8_t*)where_to + i) = *((uint8_t*)where_from + i);
}

//check if line contains valid lexems(not dividers and comments))
bool ServiceFunc::IsValidLine(const char *line) {
	bool empty = true;
	while (*line != '\0' && *line != '\n') {
		//if there are only dividers before comment
		if (empty == true && *line == ';') return false;
		if (*line != ' ' && *line != '\t') return true;
		else 							   line++;
	}
	return false;
}

bool ServiceFunc::IsLabel(const char *lexem) {
	return lexem[strlen(lexem) - 1] == ':';
}

bool ServiceFunc::IsCtrlFlow(uint32_t ctrl_flow) {
	return ctrl_flow;
}

bool ServiceFunc::IsRegister(char *argument) {
	if (argument == nullptr) return false;
	if (strstr(argument, "rax") != nullptr || 
		strstr(argument, "rbx") != nullptr ||
		strstr(argument, "rcx") != nullptr ||
		strstr(argument, "rdx") != nullptr) return true;
	else                                    return false;
}


void ASM::ParseShell(char *buff) {
	list.Parse(buff);
}
void ASM::ArgumentProcess(uint32_t arg, uint32_t *pos, uint8_t *flag, uint32_t ctrl_flow, uint32_t pass, uint32_t curline) {
	char *lexem = list.lines[curline].lexems[*pos];
	if (!ServiceFunc::IsCtrlFlow(ctrl_flow)) {
		for (auto i = 0; i < arg; i++) {
			lexem = list.lines[curline].lexems[*pos];
			if (*lexem == '[') {
				*flag |= RAM;
				if (RamProcess(++lexem, flag) != 0)
					assert(!"Ram using error");
			}
			else if (DefaultProcess(lexem, flag) != 0) {
				printf("!%c!\n", *lexem - 2);
				assert(!"Simple argument using error");
			}
			(*pos)++;
		}
	}
	else {
		CtrlFlowProcess(lexem, pass);
	}
}	

//processing functions that change Flow Order and have >=1 argument
void ASM::CtrlFlowProcess(char *label, uint32_t pass) {
	if (pass == 1) {
		uint32_t label_pos = 0;
		if ((label_pos = llabels.FindLabel(label)) == -1) {
			printf("!%s! ERROR\n", label);
			assert(!"label not found");
		}
		printf("label pos --------------------------------------------%d",label_pos);
		cdata.PushInBin(&label_pos, sizeof(label_pos));
	}
	//with the first pass we have no marks yet
	else cdata.PushInBin(&DAM, sizeof(DAM));
}

//[...] argument processing
uint32_t ASM::RamProcess(char *lexem, uint8_t *flag) {
	if (ServiceFunc::IsRegister(lexem)) {
		//printf("lexem in ram is %s\n", lexem);
		*flag |= REG;
		//by the second letter we can determine reg index.
		uint32_t reg_index = lexem[1] - 'a';
		//skip register
		lexem += 3;
		cdata.PushInBin(&reg_index, sizeof(reg_index));
		//if it is not the only argument in [...]
		if (strcmp(lexem, "]") != 0)
			return ProcessAdditional(lexem, flag, "]");
		else return 0;
	}   
	else if (isdigit(*lexem) || *lexem == '-') {
		*flag |= CONST;
		double n = strtod(lexem, &lexem);
		cdata.PushInBin(&n, sizeof(n));
		return (strcmp(lexem, "]") != 0);
	}
	else return 1;
}

uint32_t ASM::DefaultProcess(char *lexem, uint8_t *flag) {
	if (ServiceFunc::IsRegister(lexem)) {
		*flag |= REG;
		uint32_t reg_index = lexem[1] - 'a';
		printf("argument is register: !%s!, %d\n", lexem, reg_index);
		lexem += 3;
		cdata.PushInBin(&reg_index, sizeof(reg_index));
		if (strcmp(lexem, "\0") != 0)
			return ProcessAdditional(lexem, flag, "\0");
		else return 0;
	}
	//if it is number (either positive or negative)
	else if (isdigit(*lexem) || *lexem == '-') {
		*flag |= CONST;
		printf("argument is const: !%s!\n", lexem);
		double n = strtod(lexem, &lexem);
		//printf("rest of const is: !%s!\n", *lexem);
		cdata.PushInBin(&n, sizeof(n));
		//if first argument was a number and we have something else further.
		return (strcmp(lexem, "\0") != 0);
	}
	else return 1;
}

uint32_t ASM::ProcessAdditional(char *lexem, uint8_t *flag, const char *endline) {
	if (*lexem == '+') {
		lexem++;
		if (isdigit(*lexem) || *lexem == '-') {
			*flag |= CONST;
			double n = strtod(lexem, &lexem);
			cdata.PushInBin(&n, sizeof(n));
			return (strcmp(lexem, endline) != 0);
		}
		else return 1;
	}
	else return 1;
}
void ASM::PrintBinary() {
	cdata.BinaryOutput();
}
void ASM::PrintList() {
	list.LexemsOutput();
}
void ASM::PrintLabels() {
	llabels.LabelOutput();
}

void ASM::WriteInBin() {
	cdata.WriteBin();
}
//substracting 1 because of the cmd_name itself	(about arg > list.lines[i].nlexems - 1)		
#define DEF_CMD(name, num, arg, ctrl_flow) 					\
if (strcmp(list.lines[curline].lexems[pos], #name) == 0){	\
	printf("!%s! processing\n", list.lines[curline].lexems[pos]);\
	uint8_t flag = 0;                                       \
	auto flag_adrs = cdata.offset++;						\
	if (pass == 0)											\
		cdata.length++;/*increase lenght because of flag */	\
	pos++;													\
	if (arg > list.lines[curline].nlexems - 1) {			\
		printf("To few arguments Error");					\
		assert(arg <= list.lines[curline].nlexems);			\
	}														\
	if (arg < list.lines[curline].nlexems - 1) {			\
		printf("To much arguments Error");					\
		assert(arg >= list.lines[curline].nlexems);			\
	}														\
	flag |= num;											\
	if (arg > 0) 										    \
		ArgumentProcess(arg, &pos, &flag, ctrl_flow, pass, curline);\
	else printf("no arguments"); \
	cdata.PushByAdrs(flag_adrs, &flag, sizeof(flag));       \
	}														\
else   

void ASM::TranslateBin() {
	//2 passing compilation
	for (auto pass = 0; pass < PASSNUM; pass++) {
		printf("!!!!!!%d", pass);
		cdata.offset = 0;
		for (auto curline = 0; curline < list.nlines; curline++) {
			printf("line = %d\n", curline);
			uint32_t pos = 0;
			uint32_t cmd_num = 0;
			DEF_CMD(IN, 1, 0, 0)
			DEF_CMD(OUT, 2, 0, 0)
			DEF_CMD(PUSH, 3, 1, 0)
			DEF_CMD(POP, 4, 1, 0)
			DEF_CMD(ADD, 5, 0, 0)	
			DEF_CMD(SUB, 6, 0, 0)
			DEF_CMD(MUL, 7, 0, 0)
			DEF_CMD(DIV, 8, 0, 0)
			DEF_CMD(POW, 9, 0, 0)
			DEF_CMD(SQRT, 10, 0, 0)
			DEF_CMD(SIN, 11, 0, 0)
			DEF_CMD(COS, 12, 0, 0)
			DEF_CMD(HLT, 13, 0, 0)
			DEF_CMD(JMP, 14, 1, 1)
			DEF_CMD(JB, 15, 1, 1)
			DEF_CMD(JBE, 16, 1, 1)
			DEF_CMD(JE, 17, 1, 1)
			DEF_CMD(JAE, 18, 1, 1)
			DEF_CMD(JA, 19, 1, 1)
			DEF_CMD(JNE, 20, 1, 1)
			DEF_CMD(JT, 21, 1, 1)
			DEF_CMD(CALL, 22, 1, 1)
			DEF_CMD(RET, 23, 0, 1)
			DEF_CMD(EQ, 24, 0, 0)
			DEF_CMD(NEQ, 25, 0, 0)
			DEF_CMD(BEQ, 26, 0, 0)
			DEF_CMD(B, 27, 0, 0)
			DEF_CMD(AEQ, 28, 0, 0)
			DEF_CMD(A, 29, 0, 0)
			if (ServiceFunc::IsLabel(list.lines[curline].lexems[pos]) && pass == 0) 
				llabels.PushLabel(list.lines[curline].lexems[pos], cdata.offset);
			//printf("DEF is over!----------------------------------------------------------");
		}
	}
}
void ASM::MainProcess() {
	ASM::PrintList();
	ASM::TranslateBin();
	ASM::PrintLabels();
	ASM::WriteInBin();
	ASM::PrintBinary();
}



/*int main() {
	ASM::MainProcess();
	printf("ok");
	return 0;
}*/