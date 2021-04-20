
#include "../classes/proc.hpp"

BinCmd::BinCmd(const char *fname) : offset(0) {
    FILE* fin = fopen(fname, "rb");
    assert(fin);
    fseek(fin, 0L, SEEK_END);
    //+1 for \0
    offset = 0;
    length = ftell(fin); 
    fseek(fin, 0L, SEEK_SET);
    bcode = (uint8_t *)calloc(length, sizeof(uint8_t));
    fread(bcode, sizeof(uint8_t), length, fin);
    fclose(fin);
}

BinCmd::~BinCmd() {
    free(bcode);
}

void BinCmd::BinOutput() {
    for (auto i = 0; i < length; i++)
        printf("%d)%d ", i, bcode[i]);
}

template <typename T>
Stack<T>::Stack(): capacity(DEFSIZE), size(0) {
    stack = (T*)calloc(capacity, sizeof(T));
} 
template <typename T>
Stack<T>::~Stack() {
    free(stack);
}
template <typename T>
T Stack<T>::PeekTop() const {
    assert(size > 0);
    return stack[size - 1];
}
template <typename T>
T Stack<T>::Pop() {
    assert(size > 0);
    return stack[--size];
}
template <typename T>
void Stack<T>::Push(T val) {
    if (size >= capacity) {
        capacity *= 2;
        stack = (T*)realloc(stack, size * sizeof(T));
        assert(stack != nullptr);
    }
    stack[size++] = val;
}
template <typename T>
void Stack<T>::StackDump() {
    printf("\nSTACK DUMPING\n");
    for (uint32_t i = 0; i < size; i++)
        printf("%f \n", stack[i]);
}

void Processor::StackOut() {
    stk.StackDump();
} 

#define IN_FUNC {        \
    double tmp = 0;      \
    scanf("%lg", &tmp);  \
    stk.Push(tmp);       \
    cdata.offset++;      \
}

#define OUT_FUNC {                  \
    fprintf(file, "%lg", stk.PeekTop());\
    cdata.offset++;                 \
}

#define PUSH_FUNC {                                                         \
    uint32_t tmp_byte = cdata.offset++;                                     \
    if ((cdata.bcode[tmp_byte] & RAM) != 0) {                               \
        uint32_t ram_index = 0;                                             \
        if (cdata.bcode[tmp_byte] & REG != 0) {                             \
            uint32_t tmp_reg = 0;                                           \
            memcpy(&tmp_reg, cdata.bcode + cdata.offset, sizeof(tmp_reg));  \
            ram_index += (uint32_t)registers[tmp_reg];                      \
            cdata.offset += sizeof(tmp_reg);                                \
        }                                                                   \
        if ((cdata.bcode[tmp_byte] & CONST) != 0) {                         \
            double tmp_val = 0;                                             \
            memcpy(&tmp_val, cdata.bcode + cdata.offset, sizeof(tmp_val));  \
            ram_index += (uint32_t)tmp_val;                                 \
            cdata.offset += sizeof(tmp_val);                                \
        }                                                                   \
        stk.Push(ram[ram_index]);                                           \
    }                                                                       \
    else {                                                                  \
        /*if (cdata.offset == 261) */                                       \
            /*printf("here, %d\n", cdata.bcode[tmp_byte] & CONST); */       \
        double def_val = 0;                                                 \
        if ((cdata.bcode[tmp_byte] & REG) != 0) {                           \
            uint32_t tmp_reg = 0;                                           \
            memcpy(&tmp_reg, cdata.bcode + cdata.offset, sizeof(tmp_reg));  \
            def_val += registers[tmp_reg];                                  \
            cdata.offset += sizeof(tmp_reg);                                \
        }                                                                   \
        if ((cdata.bcode[tmp_byte] & CONST) != 0) {                           \
            double tmp_val = 0;                                             \
            memcpy(&tmp_val, cdata.bcode + cdata.offset, sizeof(tmp_val));  \
            def_val += tmp_val;                                             \
            cdata.offset += sizeof(tmp_val);                                \
            printf("in push, const, offset = %d\n", cdata.offset);          \
        }                                                                   \
        stk.Push(def_val);                                                  \
    }                                                                       \
}                                                               

#define POP_FUNC {                                                          \
    uint32_t tmp_byte = cdata.offset++;                                      \
    if ((cdata.bcode[tmp_byte] & RAM) != 0) {                                 \
        uint32_t ram_index = 0;                                             \
        if ((cdata.bcode[tmp_byte] & REG) != 0) {                             \
            uint32_t tmp_reg = 0;                                           \
            /*get reg num*/                                                 \
            memcpy(&tmp_reg, cdata.bcode + cdata.offset, sizeof(tmp_reg));  \
            /*add value from register*/                                     \
            ram_index += registers[tmp_reg];                                \
            cdata.offset += sizeof(tmp_reg);                                \
        }                                                                   \
        if ((cdata.bcode[tmp_byte] & CONST) != 0) {                           \
            double tmp_reg = 0;                                             \
            memcpy(&tmp_reg, cdata.bcode + cdata.offset, sizeof(tmp_reg));  \
            /*add const value to index*/                                    \
            ram_index += (uint32_t)tmp_reg;                                 \
            cdata.offset += sizeof(tmp_reg);                                \
        }                                                                   \
        ram[ram_index] = stk.Pop();                                         \
    }                                                                       \
    else {                                                                  \
        if ((cdata.bcode[tmp_byte] & REG) != 0) {                           \
            /*reg takes 4 bytes . tmp is uint32_t*/                         \
            uint32_t tmp_reg = 0;                                           \
            memcpy(&tmp_reg, cdata.bcode + cdata.offset, sizeof(tmp_reg));  \
            /*push in register from stack*/                                 \
            registers[tmp_reg] = stk.Pop();                                 \
            cdata.offset += sizeof(tmp_reg);                                \
        }                                                                   \
        else assert(!"unknown argument in POP_FUNC");                       \
    }                                                                       \
}

#define ADD_FUNC {             \
    double rhs = stk.Pop();    \
    double lhs = stk.Pop();    \
    stk.Push(lhs + rhs);       \
    cdata.offset++;            \
}

#define SUB_FUNC {              \
    double rhs = stk.Pop();     \
    double lhs = stk.Pop();     \
    stk.Push(lhs - rhs);        \
    cdata.offset++;             \
}

#define MUL_FUNC {              \
    double rhs = stk.Pop();     \
    double lhs = stk.Pop();     \
    stk.Push(lhs * rhs);        \
    cdata.offset++;             \
}

#define DIV_FUNC {                      \
    double denumerator = stk.Pop();     \
    double numerator = stk.Pop();       \
    assert(abs(denumerator) > EPS);     \
    stk.Push(numerator / denumerator);  \
    cdata.offset++;                     \
}

#define POW_FUNC {                      \
    double rhs = stk.Pop();             \
    double lhs = stk.Pop();             \
    stk.Push(pow(lhs, rhs));            \
    cdata.offset++;                     \
}

#define SQRT_FUNC {                     \
    double arg = stk.Pop();             \
    assert(arg > 0);                    \
    stk.Push(sqrt(arg));                \
    cdata.offset++;                     \
} 

#define SIN_FUNC {                      \
    double arg = stk.Pop();             \
    stk.Push(sin(arg));                 \
    cdata.offset++;                     \
}

#define COS_FUNC {                      \
    double arg = stk.Pop();             \
    stk.Push(cos(arg));                 \
    cdata.offset++;                     \
}

#define EQ_FUNC {                       \
    double rhs = stk.Pop();             \
    double lhs = stk.Pop();             \
    stk.Push(abs(rhs - lhs) < EPS);     \
    cdata.offset++;                     \
}

#define NEQ_FUNC {                      \
    double rhs = stk.Pop();             \
    double lhs = stk.Pop();             \
    stk.Push(abs(rhs - lhs) >= EPS);    \
    cdata.offset++;                     \
}

//lsh <= rhs below or equal
#define BEQ_FUNC {                      \
    double rhs = stk.Pop();             \
    double lhs = stk.Pop();             \
    stk.Push(abs(rhs - lhs) < EPS || lhs - rhs < 0);\
    cdata.offset++;                     \
}
//lhs < rhs
#define B_FUNC {                        \
    double rhs = stk.Pop();             \
    double lhs = stk.Pop();             \
    stk.Push(lhs - rhs < 0);            \
    cdata.offset++;                     \
}

//lhs > rhs above
#define A_FUNC {                        \
    double rhs = stk.Pop();             \
    double lhs = stk.Pop();             \
    stk.Push(lhs - rhs > 0);            \
    cdata.offset++;                     \
}

//lhs >= rhs above or equal
#define AEQ_FUNC {                      \
    double rhs = stk.Pop();             \
    double lhs = stk.Pop();             \
    stk.Push(abs(rhs - lhs) < EPS || lhs - rhs > 0);\
    cdata.offset++;                     \
}

#define JMP_FUNC {                                                      \
    cdata.offset++;                                                     \
    uint32_t flow_index = 0;                                            \
    memcpy(&flow_index, cdata.bcode + cdata.offset, sizeof(flow_index));\
    cdata.offset = flow_index;                                          \
}

#define JB_FUNC {                                                           \
    cdata.offset++;                                                         \
    double rhs = stk.Pop();                                                 \
    double lhs = stk.Pop();                                                 \
    if (lhs < rhs) {                                                        \
        uint32_t flow_index = 0;                                            \
        memcpy(&flow_index, cdata.bcode + cdata.offset, sizeof(flow_index));\
        cdata.offset = flow_index;                                          \
    }                                                                       \
    else cdata.offset += sizeof(uint32_t);                                  \
}

#define JBE_FUNC {                                                          \
    cdata.offset++;                                                         \
    double rhs = stk.Pop();                                                 \
    double lhs = stk.Pop();                                                 \
    if (lhs <= rhs) {                                                       \
        uint32_t flow_index = 0;                                            \
        memcpy(&flow_index, cdata.bcode + cdata.offset, sizeof(flow_index));\
        cdata.offset = flow_index;                                          \
    }                                                                       \
    else cdata.offset += sizeof(uint32_t);                                  \
}

#define JA_FUNC {                                                           \
    cdata.offset++;                                                         \
    double rhs = stk.Pop();                                                 \
    double lhs = stk.Pop();                                                 \
    if (lhs > rhs) {                                                        \
        uint32_t flow_index = 0;                                            \
        memcpy(&flow_index, cdata.bcode + cdata.offset, sizeof(flow_index));\
        cdata.offset = flow_index;                                          \
    }                                                                       \
    else cdata.offset += sizeof(uint32_t);                                  \
}

#define JAE_FUNC {                                                          \
    cdata.offset++;                                                         \
    double rhs = stk.Pop();                                                 \
    double lhs = stk.Pop();                                                 \
    if (lhs >= rhs) {                                                       \
        uint32_t flow_index = 0;                                            \
        memcpy(&flow_index, cdata.bcode + cdata.offset, sizeof(flow_index));\
        cdata.offset = flow_index;                                          \
    }                                                                       \
    else cdata.offset += sizeof(uint32_t);                                  \
}

#define JE_FUNC {                                                           \
    /*go to label bytes*/                                                   \
    cdata.offset++;                                                         \
    double rhs = stk.Pop();                                                 \
    double lhs = stk.Pop();                                                 \
    if (lhs == rhs) {                                                       \
        uint32_t flow_index = 0;                                            \
        memcpy(&flow_index, cdata.bcode + cdata.offset, sizeof(flow_index));\
        cdata.offset = flow_index;                                          \
    }                                                                       \
    else cdata.offset += sizeof(uint32_t);                                  \
}

#define JNE_FUNC {                                                          \
cdata.offset++;                                                             \
double rhs = stk.Pop();                                                     \
double lhs = stk.Pop();                                                     \
if (lhs != rhs) {                                                           \
    uint32_t flow_index = 0;                                                \
    memcpy(&flow_index, cdata.bcode + cdata.offset, sizeof(flow_index));    \
    cdata.offset = flow_index;                                              \
}                                                                           \
else cdata.offset += sizeof(uint32_t);                                      \
}

#define CALL_FUNC {                                                     \
    /*go to label bytes */                                              \
    cdata.offset++;                                                     \
    uint32_t flow_index = 0;                                            \
    memcpy(&flow_index, cdata.bcode + cdata.offset, sizeof(flow_index));\
    /*remember place from where we have gone to another function*/      \
    stkcall.Push(cdata.offset + sizeof(flow_index));                    \
    /*go by label adress*/                                              \
    cdata.offset = flow_index;                                          \
}

#define RET_FUNC {                  \
    cdata.offset = stkcall.Pop();   \
}                                   \


#define DEF_CMD(name, num, arg, ctrl_flow)        \
    if ((cdata.bcode[cdata.offset] & MASK) == num)\
        name##_FUNC                               \
    else

void Processor::SetOutput(const char *fname) {
    output = fname;
}

void Processor::Execute() {
    FILE *file = fopen(output, "w");
    //13 is a HLT code DEF_CMD(HLT, 13, 0, 0)
    while (cdata.bcode[cdata.offset] != 13 && cdata.offset < cdata.length) {
        //printf("offset: !%d!, %d\n", cdata.offset, cdata.bcode[cdata.offset]);
        //check flag
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
        DEF_CMD(JMP, 14, 1, 1)
        DEF_CMD(JB, 15, 1, 1)
        DEF_CMD(JBE, 16, 1, 1)
        DEF_CMD(JE, 17, 1, 1)
        DEF_CMD(JAE, 18, 1, 1)
        DEF_CMD(JA, 19, 1, 1)
        DEF_CMD(JNE, 20, 1, 1)
        DEF_CMD(CALL, 22, 1, 1)
        DEF_CMD(RET, 23, 0, 1)
        DEF_CMD(EQ, 24, 0, 0)
        DEF_CMD(NEQ, 25, 0, 0)
        DEF_CMD(BEQ, 26, 0, 0)
        DEF_CMD(B, 27, 0, 0)
        DEF_CMD(AEQ, 28, 0, 0)
        DEF_CMD(A, 29, 0, 0)

        assert(!"unknown command. execution error\n");
    }
    fclose(file);
    free(ram);
}

void Processor::BinPrint() {
    cdata.BinOutput();
}
void Processor::MainProcess() {
    Processor::BinPrint();
    Processor::Execute();
}

BinCmd Processor::cdata("binary.bin");
Stack<double> Processor::stk;
Stack<uint32_t> Processor::stkcall;
double *Processor:: ram = (double *)calloc(DEFSIZE, sizeof(double));
double Processor::registers[4] = {0,0,0,0};

//template class Stack<double>;

/*int main() {
    Processor::MainProcess();
    printf("ok");
    return 0;
}*/