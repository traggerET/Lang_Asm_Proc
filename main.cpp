#include "classes/Language.hpp"
#include "classes/cleanAsm.hpp"
#include "classes/proc.hpp"

/*BinCmd Processor::cdata("binary.bin");
Stack<double> Processor::stk;
Stack<uint32_t> Processor::stkcall;
double *Processor:: ram = (double *)calloc(DEFSIZE, sizeof(double));
double Processor::registers[4] = {0,0,0,0};*/
const char *Processor::output = nullptr;

int main(int argc, char *argv[]) {
    if (argc == 2) {
        Lexer::SetSource(argv[1]);
        Processor::SetOutput("writing.txt");
    }
    else if (argc == 1) {
        Lexer::SetSource("fact.txt");
        Processor::SetOutput("writing.txt");
    }
    else if (argc == 3) {
        Lexer::SetSource(argv[1]);
        Processor::SetOutput(argv[2]);
    }
    else assert(!"wrong argnnum");
    Lexer::ProcessLexer();
    Tree tree;
    Graphviz::drawGraph(tree.GetRoot(), "newfiln.png");
    Translator::DoAsm(tree.GetRoot(), "asemb.txt");
    ASM::ParseShell(ServiceFunc::ReadFile("asemb.txt"));
    ASM::MainProcess();
    Processor::MainProcess();
    return 0;
}