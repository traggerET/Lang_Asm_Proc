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

using std::vector;

enum LEXEMCODE {
    IF = 214,
    WHILE,
    ELSE,
    DOUBLE,
    FUNCTION,
    RETURN,
    PRINT,
    SCAN,
    TERMINAL,
    PLUS,
    MINUS,
    MULTIP,
    DIVIDE,
    POWER,
    OPENBR,
    CLOSEBR,
    OPENFIGURED,
    CLOSEFIGURED,
    BELOWEQUAL,
    ABOVEEQUAL, 
    EQUAL,
    NOTEQUAL,
    ABOVE,
    BELOW,
    ASSIGN,
    COMMA,
    EXTRA,
    DEFAULT,
    LINKER
};
struct ServiceLexemData {
    const char *SERVICELEXEM;
    const LEXEMCODE CODE;

};

struct Serv {

    static constexpr ServiceLexemData KNOWNSERVICENAMES[] = {
        {"if",       IF},
        {"while",    WHILE},
        {"else",     ELSE},
        {"double",   DOUBLE},
        {"function", FUNCTION},
        {"return",   RETURN},
        {"print", PRINT},
        {"scan", SCAN},
        {";",  TERMINAL},
        {"+",  PLUS},
        {"-",  MINUS},
        {"*",  MULTIP},
        {"/",  DIVIDE},
        {"^",  POWER},
        {"(",  OPENBR},
        {")",  CLOSEBR},
        {"{",  OPENFIGURED},
        {"}",  CLOSEFIGURED},
        {"<=", BELOWEQUAL},
        {">=", ABOVEEQUAL},
        {"==", EQUAL},
        {"!=", NOTEQUAL},
        {">",  ABOVE},
        {"<",  BELOW},
        {"=",  ASSIGN},
        {",",  COMMA},//!Stop!
        {"\0", EXTRA},
        {"Linker", LINKER},
    };
    static constexpr size_t size = 26;
};
enum LEXEMTYPES {
    IDENTIFIER = 50,
    NUMBER,
    SERVICE,
};


namespace ServiceFunc {
    char* ReadFile(const char* fname);
    void FindLineAndPrint(char *buff_begin, size_t line);
}

struct Lexem {
    LEXEMCODE CODE;
    LEXEMTYPES type;
    unsigned nline;
    unsigned ncolomn;
    char *value;

    bool IsEND() const;
    bool IsID() const;
    bool IsNumber() const;
    bool IsService(LEXEMCODE REQUESTED) const;
    bool IsCompare() const;
    Lexem();
    Lexem(LEXEMCODE NCODE, LEXEMTYPES NTYPE, unsigned line, unsigned colomn, const char *nvalue);
    Lexem(const Lexem &other);
    Lexem &operator=(const Lexem &other);
    ~Lexem();

};

class Lexer {
public:
    static void PrintTokens();
    static void ProcessLexer();
    static vector<Lexem> &GetLexems();
    static char *GetBuffBegin();
private:
    static unsigned nline;
    static unsigned ncolomn;
    //ptr to the beginning of code.
    //it will help us to find appropriate line in case of error
    //and to output it
    static char *buff_begin;
    static char *buff;
    static vector<Lexem> Lexems;
    static bool IsComment();
    static void SkipComments();
    static void SkipSpaces();
    static void LexicalError();
    static void ProcessDigit(char *begin);
    static int ProcessWord(char *begin);
};


class Node {
    private:
    Node *left;
    Node *right;
    Lexem current;
    public:
    Node();
    
    Node(const Lexem &newcurrent, Node *newleft, Node *newright);
    ~Node();
    Lexem &GetLexem();
    void SetLeft(Node *new_left);
    void SetRight(Node *new_right);
    Node **GetLeftPtr();
    Node **GetRightPtr();

    Node *GetLeft();
    Node *GetRight();
         
};

class Tree {
    private:
    Node *root;
    //shows number of current Lexem item in vector
    size_t idx;
    
    bool IsEnd()        const;
    bool IsIdentifier() const;
    bool IsCmp()        const;
    bool IsNMR()        const;
    bool IsServ(LEXEMCODE REQUESTED) const;
    LEXEMTYPES Type()   const ;
    LEXEMCODE Code()    const;
    size_t Line()       const;
    size_t Colomn()     const;
    const char *Value() const;
    Lexem &GetLexem();

    void SyntaxError(const char *func);
    std::optional<Node*> GetGram();
    std::optional<Node*> GetProg();
    std::optional<Node*> GetDefi();
    std::optional<Node*> GetArgs();
    std::optional<Node*> GetComp();
    std::optional<Node*> GetStat();
    std::optional<Node*> GetPrint();
    std::optional<Node*> GetScan();
    std::optional<Node*> GetInit();
    std::optional<Node*> GetAssg();
    std::optional<Node*> GetJump();
    std::optional<Node*> GetIf();
    std::optional<Node*> GetWhile();
    std::optional<Node*> GetExpr();
    std::optional<Node*> GetSimp();
    std::optional<Node*> GetTerm();
    std::optional<Node*> GetPrim();
    std::optional<Node*> GetCall();
    std::optional<Node*> GetVar();
    std::optional<Node*> GetNum();

    public:
    Node *GetRoot();
    Tree();
    ~Tree();
};

namespace Graphviz {
    const char* DOT_COMMAND = "dot -Tpng ";
    void systemCall(const char* filename, const char* graph_filename);
    void drawGraph(Node* node, const char* filename);
    void loadNodes(FILE* file, Node* node);
    void printNodeValue(FILE* file, Node* node);
}

class Translator {
    public:
    static void DoAsm(Node *root, const char *fname);
    private:
    static char *GetLabelName(Node *node);
    static void FunctionTranslation(Node *node, FILE *file);
    static void ProgrammTranslation(Node *node, FILE *file);
    static void InLoadArgs         (Node *node, FILE *file);
    static void OutLoadArgs        (Node *node, FILE *file);
    static void AddInTable         (char *var);
    static void ReturnTranslate    (Node *node, FILE *file);
    static void WhileTRanslate     (Node *node, FILE *file);
    static void IfTranslate        (Node *node, FILE *file);
    static void AssignTranslate    (Node *node, FILE *file);
    static void PrintTranslate     (Node *node, FILE *file);
    static void ScanTranslate      (Node *node, FILE *file);
    static void StatTranslation    (Node *node, FILE *file);
    static void CallTranslate      (Node *node, FILE *file);
    static void VarTranslate       (Node *node, FILE *file);
    static void ExprRecursion      (Node *node, FILE *file);
    static int  FindVarPosition    (Node *node);
    static vector<char*> vtable;
};
