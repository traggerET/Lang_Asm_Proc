#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <optional>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include "../classes/Language.hpp"

using std::vector;

//file communication is quite simple yet
//you have to write code in "fact.txt" :(
char *Lexer::buff = nullptr;
//char *Lexer::buff = ServiceFunc::ReadFile("fact.txt");
unsigned Lexer::nline = 0;
unsigned Lexer::ncolomn = 0;
char *Lexer::buff_begin = Lexer::buff;
vector<Lexem> Lexer::Lexems;
vector<char*> Translator::vtable;

char* ServiceFunc::ReadFile(const char* fname) {
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
}
void ServiceFunc::FindLineAndPrint(char *buff_begin, size_t line) {
    size_t i = 0;
    unsigned t = 0;
    while (i < line) {
        while (*(buff_begin + t) != '\n' && *(buff_begin + t) != '\0') 
            t++;
        i++;
    }
    t++; //<- skip '\n'. 
    while (*(buff_begin + t) != '\n' && *(buff_begin + t) != '\0') {
        printf("%c", *(buff_begin + t));
            t++;
    }
}

bool Lexem::IsEND() const {
    return type == SERVICE && CODE == EXTRA;
}
bool Lexem::IsID() const {
    return type == IDENTIFIER;
}
bool Lexem::IsNumber() const {
    return type == NUMBER;
}
bool Lexem::IsService(LEXEMCODE REQUESTED) const {
    //checks if current Lexem sutisfies to a requested service name
    return type == SERVICE && CODE == REQUESTED;
}
bool Lexem::IsCompare() const {
    return type == SERVICE && CODE < ASSIGN && CODE > CLOSEFIGURED;
}
Lexem::Lexem(){fprintf(stderr, "building lexem\n");};
Lexem::Lexem(LEXEMCODE NCODE, LEXEMTYPES NTYPE, unsigned line, unsigned colomn, const char *nvalue):
CODE(NCODE), type(NTYPE), nline(line), ncolomn(colomn) {
    fprintf(stderr, "constructing\n");
    fprintf(stderr, "!%s!\n", nvalue);
    value = new char[strlen(nvalue) + 1];
    //fprintf(stderr, "second constructing\n");

    strcpy(value, nvalue);
}
Lexem::Lexem(const Lexem &other) {
    fprintf(stderr, "in copy const");
    CODE = other.CODE;
    type = other.type;
    nline = other.nline;
    ncolomn = other.ncolomn;        
    value = new char[strlen(other.value) + 1];
    strcpy(value, other.value);
    fprintf(stderr, "after copying\n");
}
Lexem &Lexem::operator=(const Lexem &other){//????????????????????????????????
    if (this != &other) {
        CODE = other.CODE;
        type = other.type;
        nline = other.nline;
        ncolomn = other.ncolomn;
        fprintf(stderr, "iright away\n");
        delete[] value;
        value = new char[strlen(other.value) + 1];
        strcpy(const_cast<char*>(value), other.value);
    }
    return *this;
}
Lexem::~Lexem() {
    fprintf(stderr, "before destructing lexem\n");
    delete[] value;
    fprintf(stderr, "after destructing lexem\n");
}

void Lexer::SetSource(const char *fname) {
    Lexer::buff = ServiceFunc::ReadFile(fname);
    Lexer::buff_begin = Lexer::buff;
}

void Lexer::PrintTokens() {
    for (auto i = 0; i < Lexems.size(); i++) {
        printf("new token:\n");
        printf("    type - %d, code - %d\n", Lexems[i].type, Lexems[i].CODE);
        printf("    line - %u, colomn - %u\n", Lexems[i].nline, Lexems[i].ncolomn);
        printf("    value - %s\n", Lexems[i].value);
        printf("\n");
    }
}
void Lexer::ProcessLexer() {
    while (*buff != '\0') {
        while (IsComment() || isspace(*buff)) {
            SkipSpaces();
            SkipComments();
        }
        if (*buff == '\0')
            break;

        char *begin = buff;

        //*buff == '-' - if '-' is word begin
        //(Lexems.back().type == IDENTIFIER || -if before it stands var
        //Lexems.back().type == NUMBER) ||     -or a number 
        //(Lexems.back().type == SERVICE && Lexems.back().type == CLOSEBR) -or (expr)/func()
        if (*buff == '-' && ((Lexems.back().type == IDENTIFIER || 
            Lexems.back().type == NUMBER) || 
            (Lexems.back().type == SERVICE && Lexems.back().CODE == CLOSEBR))) {
            //printf("here5");
            if (ProcessWord(begin) != 0) 
                //printf("here4");
                LexicalError();
        }

        else if (isdigit(*buff)) {
            //printf("here");
            ProcessDigit(begin); 
        }

        else if (*buff == '-' && isdigit(*(buff + 1))) {
            //printf("here2");
            ProcessDigit(begin);
        }

        else 
            if (ProcessWord(begin) != 0) {
                printf("here3");
                LexicalError();
            }
    }
    //at the end is Specific value equal to end of prog.
    Lexems.push_back({EXTRA, SERVICE, nline, ncolomn, buff});
}
vector<Lexem> &Lexer::GetLexems() {
    fprintf(stderr, "returning array of lexems\n");
    return Lexems;
}
char *Lexer::GetBuffBegin() {
    return buff_begin;
}
bool Lexer::IsComment() {
    return *buff == '/' && *(buff + 1) == '/';
}
void Lexer::SkipComments() {
    if (IsComment()) {
        buff += 2;
        ncolomn += 2;
        while (*buff != '\n' && *buff != '\0') {
            buff++; 
            ncolomn++;
        }
    }
}
void Lexer::SkipSpaces() {
    while (isspace(*buff)) {
        if (*buff == '\n') {
            nline++; 
            ncolomn = 0;
        }
        else 
            ncolomn++;
        buff++;
    }
}
void Lexer::LexicalError() {
    printf("Lexical error in line: %d, colomn: %d\n", nline, ncolomn);
    printf("Suspected line:\n");
    ServiceFunc::FindLineAndPrint(buff_begin, nline);
    free(buff);

    assert("!Lexical error");
}
void Lexer::ProcessDigit(char *begin) {
    buff++;
    while (isdigit(*buff)) 
        buff++;
    if (*buff == '.')
        buff++;
    while (isdigit(*buff)) 
        buff++;
    char *number = new char[buff - begin + 1];
    number[buff - begin] = '\0';
    strncpy(number, begin, buff - begin);
    Lexems.push_back({DEFAULT, NUMBER, nline, ncolomn, number});
    delete[] number;
    ncolomn += buff - begin;
}
int Lexer::ProcessWord(char *begin) {
    //printf("here");
    for (auto i = 0; i < Serv::size; i++) {
        auto name = Serv::KNOWNSERVICENAMES[i].SERVICELEXEM;
        auto len = strlen(name);
        if (strncmp(buff, name, len) == 0) {
            //printf("identidied %s\n", name);
            char *tmp = new char[len + 1];
            strcpy(tmp, name);
            fprintf(stderr, "in lexer \n");
            Lexems.push_back({Serv::KNOWNSERVICENAMES[i].CODE, SERVICE, nline, ncolomn, tmp});
            fprintf(stderr, "after pushing\n");
            delete[] tmp;
            buff += len;
            ncolomn += len;
            return 0;
        }
    }
    //if service name has not been found
    if (isalpha(*buff) || *buff == '_') {
        while (isalpha(*buff) || isdigit(*buff) || *buff == '_') buff++;
        char *lexem = new char[buff - begin + 1];
        lexem[buff - begin] = '\0';
        strncpy(lexem, begin, buff - begin);
        Lexems.push_back({DEFAULT, IDENTIFIER, nline, ncolomn, lexem});

        delete[] lexem;
        ncolomn += buff - begin;
        return 0;
    }
    return 1;
}

Node::Node(): left(nullptr), right(nullptr), current({LINKER, SERVICE, 0, 0, "LINKER"}){}
Node::Node(const Lexem &newcurrent, Node *newleft, Node *newright):
left(newleft), right(newright), current(newcurrent) {
    fprintf(stderr, "in node constructing\n");
}
Node::~Node() {
    printf("deleting children");
    delete left;
    delete right;
}
Lexem &Node::GetLexem() {
    return current;
}
void Node::SetLeft(Node *new_left) {
    left = new_left;
}
void Node::SetRight(Node *new_right) {
    right = new_right;
}
Node **Node::GetLeftPtr() {
    return &left;
}
Node **Node::GetRightPtr() {
    return &right;
}
Node *Node::GetLeft() {
    return left;
}
Node *Node::GetRight() {
    return right;
}


bool Tree::IsEnd()        const {
    return Lexer::GetLexems()[idx].IsEND();
}
bool Tree::IsIdentifier() const {
    return Lexer::GetLexems()[idx].IsID();
}
bool Tree::IsCmp()        const {
    return Lexer::GetLexems()[idx].IsCompare();
}
bool Tree::IsNMR()        const {
    return Lexer::GetLexems()[idx].IsNumber();
}
bool Tree::IsServ(LEXEMCODE REQUESTED) const {
    return Lexer::GetLexems()[idx].IsService(REQUESTED);
}
LEXEMTYPES Tree::Type()   const {
    return Lexer::GetLexems()[idx].type;
}
LEXEMCODE Tree::Code()    const {
    return Lexer::GetLexems()[idx].CODE;
}
size_t Tree::Line()       const {
    return Lexer::GetLexems()[idx].nline;
}
size_t Tree::Colomn()     const {
    return Lexer::GetLexems()[idx].ncolomn;
}
const char *Tree::Value()       const {
    return Lexer::GetLexems()[idx].value;
}
Lexem &Tree::GetLexem() {
    fprintf(stderr, "getting lexem in array of lexems\n");
    return Lexer::GetLexems()[idx];
}
void Tree::SyntaxError(const char *func) {
    printf("In function: %s", func);
    printf("Syntax Error in line: %zu, colomn: %zu\n", Line(), Colomn());
    printf("value - %s, type - %d\n", Value(), Type());
    printf("Suspected line:\n");
    ServiceFunc::FindLineAndPrint(Lexer::GetBuffBegin(), Line());
}
std::optional<Node*> Tree::GetGram() {
    idx = 0;
    std::optional<Node*> troot = GetProg();
    if (!troot.has_value()) return {};

    if (!IsEnd()) {
        SyntaxError(__FUNCTION__);
        return {};
    }

    else return *troot;
}   
std::optional<Node*> Tree::GetProg() {
    size_t tmp_idx = idx;
    Node *rtmp_root = new Node;
    Node **root_ptr = &rtmp_root;

    while (IsServ(FUNCTION)) {
        idx++;

        //change actual value of left child
        std::optional<Node*> tmp = GetDefi();
        if (!tmp.has_value()) {
            delete rtmp_root;
            SyntaxError(__FUNCTION__);
            return {};
        }
        (*root_ptr)->SetLeft(*tmp);

        //create new linker in right son
        (*root_ptr)->SetRight(new Node);

        //make right child to a new "parent"
        root_ptr = (*root_ptr)->GetRightPtr();
    }
    //no function has been detected.
    if (tmp_idx == idx) {
        delete rtmp_root;
        SyntaxError(__FUNCTION__);
        return {};
    }
    return rtmp_root;
}
std::optional<Node*> Tree::GetDefi() {
    std::optional<Node*> func_t = GetVar();//GetVar();
    if (!func_t.has_value()) {
        SyntaxError(__FUNCTION__);
        return {};
    }
    Node *func = *func_t;

    if (!IsServ(OPENBR)) {
        SyntaxError(__FUNCTION__);
        delete func;
        return {};
    }
    idx++;

    std::optional<Node*> tmp = GetArgs();
    if (!tmp.has_value()) {
        delete func;
        return {};
    }
    Node *args = *tmp;
    func->SetLeft(args);

    if (!IsServ(CLOSEBR)) {
        delete func;
        SyntaxError(__FUNCTION__);
        return {};
    }
    idx++;

    std::optional<Node*> comp_t = GetComp();
    if (!comp_t.has_value()) {
        delete func;
        SyntaxError(__FUNCTION__);
        return {};
    }
    func->SetRight(*comp_t);

    return func;
}
//list of arguments accepts only variables
std::optional<Node*> Tree::GetArgs() {
    //if list of args is empty
    if (!IsIdentifier()) return new Node;

    //get first arg
    Node *arg = new Node(GetLexem(), nullptr, nullptr);
    Node **cur_arg = &arg;
    idx++;

    //get next arguments if they exist.
    while (IsServ(COMMA)) {
        idx++;
        std::optional<Node*> tnext_arg = GetVar();
        if (!tnext_arg.has_value()) {
            delete arg;
            SyntaxError(__FUNCTION__);
            return {};
        }
        Node *next_arg = *tnext_arg;

        (*cur_arg)->SetLeft(next_arg);
        cur_arg = &next_arg;
    }
    return arg;
}
std::optional<Node*> Tree::GetComp() {
    if (!IsServ(OPENFIGURED)) {
        SyntaxError(__FUNCTION__);
        return {};
    }
    idx++;

    //if function has no statemets
    if (IsServ(CLOSEFIGURED)) {
        idx++;
        return nullptr;
    }
    Node *tcomp = new Node;
    Node **comp_ptr = &tcomp;
    while (!IsServ(CLOSEFIGURED) && !IsServ(EXTRA)) {
        size_t t_idx = idx;
        std::optional<Node*> temp_stat = GetStat();
        if (!temp_stat.has_value()) {
            delete tcomp;
            return {};
        }
        Node *stat = *temp_stat;

        (*comp_ptr)->SetLeft(stat);
        (*comp_ptr)->SetRight(new Node);
        comp_ptr = (*comp_ptr)->GetRightPtr();
    }

    if (!IsServ(CLOSEFIGURED)) {
        delete tcomp;
        SyntaxError(__FUNCTION__);
    }

    idx++;
    return tcomp;
}
std::optional<Node*> Tree::GetStat() {
    size_t tmp_idx = idx;
    std::optional<Node*> tmp_node = GetExpr();
    if (!tmp_node.has_value() || !IsServ(TERMINAL)) {
        if (tmp_node.has_value()) delete *tmp_node;
        idx = tmp_idx;
        tmp_node = GetAssg();
    }

    if (!tmp_node.has_value() || !IsServ(TERMINAL)) {
        if (tmp_node.has_value()) delete *tmp_node;
        idx = tmp_idx;
        tmp_node = GetInit();
    }

    if (!tmp_node.has_value() || !IsServ(TERMINAL)) {
        if (tmp_node.has_value()) delete *tmp_node;
        idx = tmp_idx;
        tmp_node = GetJump();
    }

    if (!tmp_node.has_value() || !IsServ(TERMINAL)) {
        if (tmp_node.has_value()) delete *tmp_node;
        idx = tmp_idx;
        tmp_node = GetPrint();
    }

    if (!tmp_node.has_value() || !IsServ(TERMINAL)) {
        if (tmp_node.has_value()) delete *tmp_node;
        idx = tmp_idx;
        tmp_node = GetScan();
    }

    if (tmp_node.has_value()) {
        if (!IsServ(TERMINAL)) {
            delete *tmp_node;
            SyntaxError(__FUNCTION__);
            return {};
        }
        idx++;
        return *tmp_node;
    }

    if (!tmp_node.has_value()) {
        idx = tmp_idx;
        tmp_node = GetIf();
    }

    if (!tmp_node.has_value()) {
        idx = tmp_idx;
        tmp_node = GetWhile();
    }

    if (!tmp_node.has_value()) {
        idx = tmp_idx;
        SyntaxError(__FUNCTION__);
        return {};
    }

    return *tmp_node;
}
std::optional<Node*> Tree::GetPrint() {
    if (!IsServ(PRINT)) return {};

    Node *tmpp = new Node(GetLexem(), nullptr, nullptr);
    idx++;

    if (!IsServ(OPENBR)) {
        delete tmpp;
        return {};
    }
    idx++;

    std::optional<Node*> tmpv = GetVar();
    if (!tmpv.has_value()) {
        delete tmpp;
        return {};
    }
    tmpp->SetLeft(*tmpv);

    if (!IsServ(CLOSEBR)) {
        delete tmpp;
        return {};
    }
    idx++;
    return tmpp;
}
std::optional<Node*> Tree::GetScan() {
    if (!IsServ(SCAN)) return {};

    Node *tmps = new Node(GetLexem(), nullptr, nullptr);
    idx++;

    if (!IsServ(OPENBR)) {
        delete tmps;
        return {};
    }
    idx++;

    std::optional<Node*> tmpv = GetVar();
    if (!tmpv.has_value()) {
        delete tmps;
        return {};
    }
    tmps->SetLeft(*tmpv);

    if (!IsServ(CLOSEBR)) {
        delete tmps;
        return {};
    }
    idx++;
    return tmps;
}
std::optional<Node*> Tree::GetInit() {
    if (!IsServ(DOUBLE)) return {};
    idx++;

    std::optional<Node*> tmpv = GetVar();
    if (!tmpv.has_value()) return {};
    Node *var = *tmpv;

    if (!IsServ(ASSIGN)) {
        delete var;
        return {};
    }
    Node *init = new Node(GetLexem(),var, nullptr);
    idx++;

    std::optional<Node*> tmpe = GetExpr();
    if (!tmpe.has_value()) {
        delete init;
        return {};
    }
    Node *expr = *tmpe;
    init->SetRight(expr);
    
    return init;
}
std::optional<Node*> Tree::GetAssg() {  
    std::optional<Node*> tmpv = GetVar();

    if (!tmpv.has_value()) return {};
    Node *var = *tmpv;

    if (!IsServ(ASSIGN)) {
        delete var;
        return {};
    }
    Node *ass = new Node(GetLexem(),var, nullptr);
    idx++;

    std::optional<Node*> tmpe = GetExpr();
    if (!tmpe.has_value()) {
        delete ass;
        return {};
    }
    Node *expr = *tmpe;
    ass->SetRight(expr);
    
    return ass;
}
std::optional<Node*> Tree::GetJump() {  
    if (!IsServ(RETURN)) return {}; 
    //create left node with "return"
    Node *ret = new Node(GetLexem(), nullptr, nullptr); 
    idx++;

    std::optional<Node*> tmp = GetExpr();
    if (!tmp.has_value()) {
        delete ret;
        return {};
    }
    Node *expr = *tmp;

    ret->SetLeft(expr);
    return ret;
}
std::optional<Node*> Tree::GetIf() { 
    if (!IsServ(IF)) return {}; 

    Node *tmpif = new Node(GetLexem(), nullptr, nullptr);
    idx++;

    if (!IsServ(OPENBR)) { 
        delete tmpif;
        return {}; 
    }
    idx++;

    std::optional<Node*> tmp_cond = GetExpr();
    if (!tmp_cond.has_value()) { 
        delete tmpif;
        return {};
    }
    Node *cond = *tmp_cond;

    tmpif->SetLeft(cond);

    if (!IsServ(CLOSEBR)) {
        delete tmpif;
        return {}; 
    }
    idx++;
    
    std::optional<Node*> tmp_comp = GetComp();
    if (!tmp_comp.has_value()) {
        delete tmpif;
        return {};
    }
    Node *comp = *tmp_comp;

    Node *tmp_right = new Node;
    tmpif->SetRight(tmp_right);

    //left child is for {COMP} when IF statement is true
    tmp_right->SetLeft(comp);

    if (!IsServ(ELSE)) tmp_right->SetRight(nullptr);
    else {
        idx++;

        std::optional<Node*> opt_cond = GetComp();
        if (!opt_cond.has_value()) {
            delete tmpif;
            return {};
        }
        Node *opt = *opt_cond;

        //right child is when If statement is false
        tmp_right->SetRight(opt);
    }

    return tmpif;
}
std::optional<Node*> Tree::GetWhile() { 
    if (!IsServ(WHILE)) return {}; 
    Node *tmp_while = new Node(GetLexem(), nullptr, nullptr);
    idx++;

    if (!IsServ(OPENBR)) {
        delete tmp_while;
        return {}; 
    }
    idx++;

    std::optional<Node*> tmp_cond = GetExpr();
    if (!tmp_cond.has_value()) { 
        delete tmp_while;
        return {};
    }
    Node *cond = *tmp_cond;

    tmp_while->SetLeft(cond);

    if (!IsServ(CLOSEBR)) {
        delete cond;
        delete tmp_while;
        return {}; 
    }
    idx++;

    std::optional<Node*> tmp_comp = GetComp();
    if (!tmp_comp.has_value()) {
        delete tmp_while;
        return {};
    }
    Node *comp = *tmp_comp; 

    tmp_while->SetRight(comp);

    return tmp_while;
}
std::optional<Node*> Tree::GetExpr() {
    std::optional<Node*> t = GetSimp();
    if (!t.has_value()) return {};
    Node *expr = *t;
    Node *tmp_cmp = nullptr;

    while (IsCmp()) {
        tmp_cmp = new Node(GetLexem(), nullptr, nullptr);
        idx++;

        std::optional<Node*> rexpr_t = GetSimp();
        if (!rexpr_t.has_value()) {
            delete expr;
            delete tmp_cmp;
            return {};
        }
        Node *rexpr = *rexpr_t;

        tmp_cmp->SetLeft(expr);
        tmp_cmp->SetRight(rexpr);
        expr = tmp_cmp;
    }
    return expr;
}
std::optional<Node*> Tree::GetSimp() {
    std::optional<Node*> t = GetTerm();
    if (!t.has_value()) return {};

    Node *lsimp = *t;
    Node *tmp_term = nullptr;

    while (IsServ(PLUS) || IsServ(MINUS)) {
        tmp_term = new Node(GetLexem(), nullptr, nullptr);
        idx++;

        std::optional<Node*> rsimp_t = GetTerm();
        if (!rsimp_t.has_value()) {
            delete lsimp;
            delete tmp_term;
            return {};
        }
        Node *rsimp = *rsimp_t;

        tmp_term->SetLeft(lsimp);
        tmp_term->SetRight(rsimp);
        lsimp = tmp_term;
    }
    return lsimp;
}
std::optional<Node*> Tree::GetTerm() {
    std::optional<Node*> t = GetPrim();
    if (!t.has_value()) return {};

    Node *term = *t;
    Node *tmp_prim = nullptr;
    
    while (IsServ(MULTIP) || IsServ(DIVIDE)) {
        tmp_prim = new Node(GetLexem(), nullptr, nullptr);
        idx++;

        std::optional<Node*> rprim_t = GetPrim();
        if (!rprim_t.has_value()) {
            delete term;
            delete tmp_prim;
            return {};
        }
        Node *rprim = *rprim_t;

        tmp_prim->SetLeft(term);
        tmp_prim->SetRight(rprim);
        term = tmp_prim;
    }
    return term;
}
std::optional<Node*> Tree::GetPrim() {

    size_t t_idx = idx;

    std::optional<Node*> prim = GetNum();
    if (!prim.has_value()) {
        idx = t_idx;
        prim = GetCall();
    }
    if (!prim.has_value()) {
        idx = t_idx;
        prim = GetVar();
    }
    if (!prim.has_value()) {
        idx = t_idx;

        if (!IsServ(OPENBR)) return {};
        idx++;

        prim = GetExpr();
        if (!prim.has_value()) return {};

        if (!IsServ(CLOSEBR)) {
            delete *prim;
            return {};
        }
        idx++;
    }
    return prim;
}
std::optional<Node*> Tree::GetCall() {

    std::optional<Node*> tmp_func = GetVar();
    if (!tmp_func.has_value()) return {};

    if (!IsServ(OPENBR)) {
        fprintf(stderr, "deleting in getcall\n");
        delete *tmp_func;
        return {};
    }
    idx++;

    std::optional<Node*> tmp_args = GetArgs();

    if (!tmp_args.has_value()) {
        delete *tmp_func;
        return {};
    }
    (*tmp_func)->SetLeft(*tmp_args);
    (*tmp_func)->SetRight(nullptr);

    if (!IsServ(CLOSEBR)) {
        delete *tmp_func;
        return {};
    }
    idx++;

    return tmp_func;
}
std::optional<Node*> Tree::GetVar() {
    if (!IsIdentifier()) return {};
    //fprintf(stderr, "Above Before crash - 1\n");
    Node *tmp = new Node(GetLexem(), nullptr, nullptr);
    //fprintf(stderr, "Before crash\n");
    //fprintf(stderr, "value %s\n", tmp->GetLexem().value);
    idx++;
    return tmp;
}
std::optional<Node*> Tree::GetNum() {
    if (!IsNMR()) return {};
    Node *tmp = new Node(GetLexem(), nullptr, nullptr);
    idx++;
    return tmp;
}

Node *Tree::GetRoot() {
    return root;;
}
Tree::Tree() {
    std::optional<Node*> t = GetGram();
    if (!t.has_value()) {
        printf("Tree construction has failed\n");
        assert(!"Emergency\n");
    }
    root = *t;
    printf("Successful tree construction\n");
}
Tree::~Tree() {
    printf("deleting tree");
    delete root;
}

//Service fu
void Graphviz::systemCall(const char* filename, const char* graph_filename)
{
    char command[96] = {};

    size_t i = 0;
    while (DOT_COMMAND[i] != '\0')
    {
        command[i] = DOT_COMMAND[i];
        ++i;
    }
    size_t j = 0;
    while (filename[j] != '\0')
    {
        command[i] = filename[j++];
        ++i;
    }
    command[i++] = ' ';
    command[i++] = '-';
    command[i++] = 'o';
    command[i++] = ' ';
    j = 0;
    while (graph_filename[j] != '\0')
    {
        command[i] = graph_filename[j++];
        ++i;
    }
    
    system(command);
}
void Graphviz::drawGraph(Node* node, const char* filename)
{   
    if (node == nullptr) 
    {
        return;
    }
    FILE* file = fopen("graphPlot.txt", "w");
    assert(file != nullptr);
    fprintf(file, "digraph G{\n");
    fprintf(file, "node [shape=\"record\", style=\"solid\", color=\"blue\"];\n");
    printf("now here!\n");
    loadNodes(file, node);

    fprintf(file, "}");

    fclose(file);

    systemCall("graphPlot.txt", filename);
}
void Graphviz::loadNodes(FILE* file, Node* node)
{
    if (node == nullptr)
    {
        return;
    }
    if (node->GetLeft() != nullptr)
    {
        fprintf(file, "\"%p\":sw->\"%p\"\n", node, node->GetLeft());  // link parent and left child
        loadNodes(file, node->GetLeft());
    }
    if (node->GetRight() != nullptr)
    {
        fprintf(file, "\"%p\":se->\"%p\"\n", node, node->GetRight());  // link parent and right child
        loadNodes(file, node->GetRight());
    }
    printNodeValue(file, node);
}
void Graphviz::printNodeValue(FILE* file, Node* node)
{
    switch (node->GetLexem().type)
    {
        case IDENTIFIER:
            fprintf(file, "\"%p\" [style=\"filled\", fillcolor=\"seagreen1\", label=\"{%s}\"]\n ", 
                    node, node->GetLexem().value); 
            break;

        case NUMBER:
            fprintf(file, "\"%p\" [style=\"filled\", fillcolor=\"lightpink\", label=\"{%s}\"]\n ", 
                    node, node->GetLexem().value); 
            break;

        case SERVICE:
            //printf("value - %s, type - %d, code - %d\n", node->GetLexem().value, node->GetLexem().type, node->GetLexem().CODE);
            switch (node->GetLexem().CODE) {
                case LINKER:
                    fprintf(file, "\"%p\" [style=\"filled\", fillcolor=\"gray\", label=\"{%s}\"]\n ",
                        node, node->GetLexem().value); 
                    break;
                default:
                    //printf("value - %s, type - %d, code - %d\n", node->GetLexem().value, node->GetLexem().type, node->GetLexem().CODE);
                    fprintf(file, "\"%p\" [style=\"filled\", fillcolor=\"blue\", label=\"{%s}\"]\n ", 
                        node, node->GetLexem().value); 
                    break;
            }
            break;

        default:
            printf("ERROR IN GRAPH PAINTING\n");
            //printf("value - %s, type - %d, code - %d\n", node->GetLexem().value, node->GetLexem().type, node->GetLexem().CODE);
            break;
    }
}


void Translator::DoAsm(Node *root, const char *fname) {
    FILE *file = fopen(fname, "w"); 
    ProgrammTranslation(root, file);
    fclose(file);
}
char *Translator::GetLabelName(Node *node) {
    auto basic_len = strlen(node->GetLexem().value);
    auto number = node->GetLexem().nline;
    //(log10(number) + 1) get number of digits in number
    auto number_len = (int)floor(log10(number)) + 1;

    char *name = new char[number_len + basic_len + 1];
    strcpy(name, node->GetLexem().value);
    //right after loop name will be writen its line in source code
    char *tmp = name + basic_len;
    sprintf(tmp, "%d", number);

    return name;
}
void Translator::FunctionTranslation(Node *node, FILE *file) {
    fprintf(stderr, "in function %s\n", node->GetLexem().value);
    fprintf(file, "%s:\n", node->GetLexem().value);
    OutLoadArgs(node->GetLeft(), file);
    node = node->GetRight();
    
    while (node->GetRight() != nullptr) {
        StatTranslation(node->GetLeft(), file);
        fprintf(stderr, "after stat translation\n");
        node = node->GetRight();
        fprintf(stderr, "ending cycle\n");
    }
}
void Translator::ProgrammTranslation(Node *node, FILE *file) {
    const char *MAIN = "Main";
    fprintf(file, "PUSH 0\n");
    fprintf(file, "POP rbx\n");
    fprintf(file, "PUSH 0\n");
    fprintf(file, "POP [rbx]\n");
    fprintf(file, "CALL %s\n", MAIN);
    fprintf(file, "HLT\n");

    while (node->GetRight() != nullptr) {
        FunctionTranslation(node->GetLeft(), file);
        node = node->GetRight();
        for (auto i = 0; i < vtable.size(); i++)
            delete[] vtable[i];
        vtable.clear();
    }
}
//loading arguments in stack to be called from function
//list of arguments accepts only variables
void Translator::InLoadArgs(Node *node, FILE *file) {
    if (strcmp(node->GetLexem().value, "LINKER") == 0) return;
    while (node != nullptr) {
        int val = FindVarPosition(node);
        if (val == -1) {
            fprintf(stderr, "Unknown variable !%s! used\n", node->GetLexem().value);
            fprintf(stderr, "line - %d, colomn - %d, \n", node->GetLexem().nline, node->GetLexem().ncolomn);
            assert(!"Unknown var\n");
        }
        //dont add 2 because FindVarPosition has taken it into account
        fprintf(stderr, "pushing %d\n", val);
        fprintf(file, "PUSH [rbx+%d]\n", val);
        node = node->GetLeft();
    }
}
//loading arguments from stack to be used in function
void Translator::OutLoadArgs(Node *node, FILE *file) {
    if (strcmp(node->GetLexem().value, "LINKER") == 0) { fprintf(stderr, "no arguments\n");return;}  
    while (node != nullptr) {
        fprintf(file, "PUSH 1\n");
        fprintf(file, "PUSH [rbx+1]\n");
        fprintf(file, "ADD\n");
        fprintf(file, "POP [rbx+1]\n");
        fprintf(file, "POP [rbx+%ld]\n", vtable.size() + 2);
        AddInTable(node->GetLexem().value);
        node = node->GetLeft();
    }
}
void Translator::AddInTable(char *var) {
    vtable.push_back(nullptr);
    vtable.back() = new char[strlen(var) + 1];
    strcpy(vtable.back(), var);
    fprintf(stderr, "new var record:%s\n", vtable.back());
}
void Translator::ReturnTranslate(Node *node, FILE *file) {
    ExprRecursion(node->GetLeft(), file);
    fprintf(file, "PUSH [rbx]\n");
    fprintf(file, "POP rbx\n");
    fprintf(stderr, "before expr\n");
    //pushing value that will be returned
    fprintf(stderr, "after expr\n");
    fprintf(file, "RET\n");
}
void Translator::WhileTRanslate(Node *node, FILE *file) {
    char *lname = GetLabelName(node);
    fprintf(stderr, "label name is  %s\n", lname);
    fprintf(file, "%s_begin:\n", lname);
    ExprRecursion(node->GetLeft(), file);
    fprintf(file, "PUSH 0\n");
    fprintf(file, "JE %s_end\n", lname);
    node = node->GetRight();

    while (node->GetRight() != nullptr) {
        StatTranslation(node->GetLeft(), file);
        node = node->GetRight();
    }
    fprintf(file, "JMP %s_begin\n", lname);
    fprintf(file, "%s_end:\n", lname);

    delete[] lname;
}
void Translator::IfTranslate(Node *node, FILE *file) {
    char *lname = GetLabelName(node);
    ExprRecursion(node->GetLeft(), file);
    fprintf(file, "PUSH 0\n");
    fprintf(file, "JE %s_end\n", lname);
    Node *tmp = node; 
    node = node->GetRight()->GetLeft();

    //write commands which are executed when condition is true
    while (node->GetRight() != nullptr) {
        StatTranslation(node->GetLeft(), file);
        node = node->GetRight();
    }
    node = tmp;
    node = node->GetRight();

    //if Else statements exist.
    if (node->GetRight() != nullptr) {
        node = node->GetRight();
        fprintf(file, "JMP alt_%s_end\n", lname);
        fprintf(file, "%s_end:\n", lname);
        while (node->GetRight() != nullptr) {
            StatTranslation(node->GetLeft(), file);
            node = node->GetRight();
        }
        fprintf(file, "JMP alt_%s_end\n", lname);
        fprintf(file, "alt_%s_end:\n", lname);
    }
    else {
        //if do not exist.
        fprintf(file, "JMP %s_end\n", lname);
        fprintf(file, "%s_end:\n", lname);
    }
    delete[] lname;
}
void Translator::AssignTranslate(Node *node, FILE *file) {
    int offset = FindVarPosition(node->GetLeft());
    //if is new var
    if (offset == -1) {
        fprintf(stderr, "var not found\n");
        fprintf(file, "PUSH 1\n");
        fprintf(file, "PUSH [rbx+1]\n");
        fprintf(file, "ADD\n");
        fprintf(file, "POP [rbx+1]\n");
        char *var = node->GetLeft()->GetLexem().value;
        offset = vtable.size() + 2;
        AddInTable(var);
    }
    //result of rhs is in stack. after ExrprRecursion
    ExprRecursion(node->GetRight(), file);
    fprintf(file, "POP [rbx+%d]\n", offset); 
}
void Translator::PrintTranslate(Node *node, FILE *file) {
    int offset = FindVarPosition(node->GetLeft());

    if (offset == -1) {
        fprintf(stderr, "Unknown variable !%s! used\n", node->GetLexem().value);
        fprintf(stderr, "line - %d, colomn - %d, \n", node->GetLexem().nline, node->GetLexem().ncolomn);
        assert(!"Unknown var\n");
    }

    fprintf(file, "PUSH [rbx+%d]\n", offset); 
    fprintf(file, "OUT\n"); 
    fprintf(file, "POP [rbx+%d]\n", offset); 
}
void Translator::ScanTranslate(Node *node, FILE *file) {
    int offset = FindVarPosition(node->GetLeft());

    if (offset == -1) {
        fprintf(stderr, "Unknown variable !%s! used\n", node->GetLexem().value);
        fprintf(stderr, "line - %d, colomn - %d, \n", node->GetLexem().nline, node->GetLexem().ncolomn);
        assert(!"Unknown var\n");
    }
    fprintf(file, "IN\n"); 
    fprintf(file, "POP [rbx+%d]\n", offset); 
}
void Translator::StatTranslation(Node *node, FILE *file) {
    switch (node->GetLexem().type) {
        case SERVICE: 
            switch (node->GetLexem().CODE) {

                case RETURN: 
                    fprintf(stderr, "in return\n");
                    ReturnTranslate(node, file);
                    break;

                case PRINT:
                    PrintTranslate(node, file);
                    break;

                case SCAN:
                    ScanTranslate(node, file);
                    break;

                case WHILE: 
                    fprintf(stderr, "in while\n");
                    WhileTRanslate(node, file);
                    break;

                case IF: 
                    IfTranslate(node, file);
                    break;

                case ASSIGN: 
                    fprintf(stderr, "in assign\n");
                    AssignTranslate(node, file);
                    break;
                
                default:
                    ExprRecursion(node, file);
                    break;
            }
            break;
        default: 
            ExprRecursion(node, file);
            break;
    }
}
int Translator::FindVarPosition(Node *node) {
    fprintf(stderr, "findvar: looking for %s\n", node->GetLexem().value);
    for (auto i = 0; i < vtable.size(); i++) {
        fprintf(stderr, "findvar: cur is %s\n", vtable[i]);
        if (strcmp(node->GetLexem().value, vtable[i]) == 0)
            return i + 2;
    }
    //if not found
    return -1;
}
void Translator::CallTranslate(Node *node, FILE *file) {
    InLoadArgs(node->GetLeft(), file);

    //shifting rbx further and remembering its previous position
    fprintf(file, "PUSH rbx\n");
    fprintf(file, "PUSH rbx\n");
    fprintf(file, "PUSH [rbx+1]\n");
    fprintf(file, "ADD\n");
    fprintf(file, "POP rbx\n");
    fprintf(file, "POP [rbx]\n");

    fprintf(file, "CALL %s\n", node->GetLexem().value);
}
void Translator::VarTranslate(Node *node, FILE *file) {
    int offset = FindVarPosition(node);
    if (offset == -1) {
        fprintf(stderr, "Unknown variable !%s! used\n", node->GetLexem().value);
        fprintf(stderr, "line - %d, colomn - %d, \n", node->GetLexem().nline, node->GetLexem().ncolomn);
        assert(!"Unknown var\n");
    }
    //here we dont add 2 because  FindVarPosition has taken it into account
    fprintf(file, "PUSH [rbx+%d]\n", offset);
}
void Translator::ExprRecursion(Node *node, FILE *file) {
    if (node == nullptr) return;
    fprintf(stderr, "var is ::::%s\n", node->GetLexem().value);
    //if (strcmp(node->GetLexem().value, "<=") == 0)
    //    fprintf(stderr, "logic operator: code - %d, type - %d\n", node->GetLexem().CODE, node->GetLexem().type);
    switch (node->GetLexem().type) {
        case NUMBER:
            fprintf(file, "PUSH %s\n", node->GetLexem().value);
            break;

        case IDENTIFIER:
            //if it's function then left child is not nullptr.
            if (node->GetLeft() != nullptr) 
                CallTranslate(node, file);
            //if it's var - both children are nullptr
            else {
                VarTranslate(node, file);
            }
            break;
        case SERVICE:
            ExprRecursion(node->GetLeft(), file);
            ExprRecursion(node->GetRight(), file);
            switch(node->GetLexem().CODE) {
                case PLUS:
                    fprintf(file, "ADD\n");
                    break;
                case MINUS:
                    fprintf(file, "SUB\n");
                    break;
                case MULTIP:
                    fprintf(file, "MUL\n");
                    break;
                case DIVIDE:
                    fprintf(file, "DIV\n");
                    break;
                case POWER:
                    fprintf(file, "POW\n");
                    break;
                case EQUAL:
                    fprintf(file, "EQ\n");
                    break;
                case ABOVEEQUAL:
                    fprintf(file, "AEQ\n");
                    break;
                case BELOW:
                    fprintf(file, "B\n");
                    break;
                case BELOWEQUAL:
                    fprintf(stderr, "belowequal\n");
                    fprintf(file, "BEQ\n");
                    break;
                case NOTEQUAL:
                    fprintf(file, "NEQ\n");
                    break;
                case ABOVE:
                    fprintf(file, "A\n");
                    break;
                default:
                    fprintf(stderr, "Unknown operand in ExprRecursion\n");
                    break;
            }
            break;
    }
}



/*int main() {
    Lexer::ProcessLexer();
    //Lexer::PrintTokens();
    Tree tree;
    Graphviz::drawGraph(tree.GetRoot(), "newfiln.png");
    Translator::DoAsm(tree.GetRoot(), "asemb.txt");
    printf("sdnio");
    return 0;
}*/
