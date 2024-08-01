#pragma once
#include "Types.hpp"
#include "Token.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
class VirtualMachine;


enum OpCode
{
    ZERO = 0,
    PUSH,
    POP,
    CONST,
    RETURN,
    HALT,
    PRINT,
    NOW,
    FRAME,
    PROGRAM,

    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,

    MOD,
    POWER,
    NEGATE,

    EQUAL,
    NOT_EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,

    TRUE,
    FALSE,
    NOT,
    AND,
    OR,
    XOR,
    INC,
    DEC,
    SHL,
    SHR,



    GLOBAL_DEFINE,
    GLOBAL_GET,
    GLOBAL_ASSIGN,

    
    LOCAL_GET,
    LOCAL_SET,



    SWITCH,
    CASE,
    SWITCH_DEFAULT,

    DUP,
    EVAL_EQUAL,

    JUMP_BACK,

    LOOP_BEGIN,
    LOOP_END,

    BREAK,
    CONTINUE,
    
    DROP,
    CALL,
    CALL_SCRIPT,
    CALL_PROCESS,
    RETURN_DEF,
    RETURN_PROCESS,

    
    NIL,

    JUMP,
    JUMP_IF_FALSE,
    JUMP_IF_TRUE,
    COUNT,
};

class Task;



#define MAX_FRAMES 64
#define STACK_MAX (MAX_FRAMES * UINT8_MAX)


static const int RUNNING = 0;
static const int FINISHED = 1;
static const int ABORTED = 2;
static const int TERMINATED = 3;
static const int OK = 4;


struct Scope 
{


    int level;
    HashTable<Value> variables;
    Scope();
    Scope( int level);
    virtual ~Scope();
    bool define(const char *name, Value value);
    void insert(const char *name, Value value);
    bool get(const char *name, Value &value);
    bool contains(const char* name) const;
    bool assign(const char* name, Value value);


    void print();
    void clear();
};

struct  Local
{
    char name[128]{'\0'};
    u32 len;
    int depth;
    bool isArg;
} ;

struct Frame
{
    Task  *task;
    u8    *ip;
    Value *slots;
};



class Task : public Traceable
{
private:
    friend class VirtualMachine;
    friend class Parser;

    bool PanicMode;

    
    
    
    Vector<u8> codes;

    int loopStart{-1};
    int exitJump{-1};

    int breakJumpCount{-1};
    int breakJumps[UINT8_MAX]{-1};
    int frameDepth;

    


    Value stack[STACK_MAX];
    Value* stackTop;

    int scopeDepth;

    bool mainTask;


 
    u8 argsCount;



    void beginScope();
    void exitScope(int line);
    
    bool push(Value v);
    Value pop();
    Value peek(int offset = 0);
    Value top();
    void pop(u32 count);
    void PrintStack();

   // u8 read_byte();
   //u16 read_short();

    u8 makeConstant( Value value);
    void writeByte(u8 byte, int line);
    void writeBytes(u8 byte1, u8 byte2,int line);
    void writeConstant( Value value,int line);
    
    void writeConstantVar(const char *name, Value value, int line);

    void disassembleCode(const char *name);
    u32 disassembleInstruction(u32 offset);
    u32 constantInstruction(const char *name, u32 offset);
    u32 simpleInstruction(const char *name, u32 offset);
    u32 byteInstruction(const char *name, u32 offset);
    u32 jumpInstruction(const char *name, u32 sign, u32 offset);
    u32 varInstruction(const char *name, u32 offset);

    

    

    u8 op_add();
    u8 op_mod(int line);
    u8 op_not_equal();
    u8 op_less();
    u8 op_greater();
    u8 op_less_equal();
    u8 op_greater_equal();
    u8 op_xor();

   

protected:
    String name;
    u64 ID;
    Frame frames[MAX_FRAMES];
    int frameCount;
    Local locals[UINT8_MAX];
    int localCount;
    
    bool m_done;
    Task *parent;
    VirtualMachine *vm;
    bool is_main;
    bool isReturned;
    Chunk *chunk;
    Vector<Value> constants;

    int declareVariable(const String &string,bool isArg=false);
    int addLocal(const char* name,u32 len,bool isArg=false);
    int resolveLocal(const String &string);
    bool setLocalVariable(const String &string, int index);
    void set_process();

public:
    Task(VirtualMachine *vm, const char *name);
    virtual ~Task();

    void init_frames();

   
    u8 Run();
    void write_byte(u8 byte, int line);

    

    u8 addConst(Value v);
    u8 addConstString(const char *str);
    u8 addConstNumber(double number);

    virtual void create(){};
    virtual void remove(){};
    virtual void update(){};

    bool IsDone();
    void Done();
    bool IsMain() const { return is_main; }

};


struct FunctionObject : public Task
{
    int arity;
    
    FunctionObject(VirtualMachine *vm, const char *name);
    
};

const int IID    = 0;
const int IX     = 1;
const int IY     = 2;
const int IGRAPH = 3;


struct Instance 
{
    u32 ID;
    s32 locals[32];
    String name;
    Instance *father;
    Instance *son;
    Instance *smallBrother;
    Instance *bigBrother;
    void *user_data;
    Instance()
    {
       
        name = "";
        father = nullptr;
        son = nullptr;
        smallBrother = nullptr;
        bigBrother = nullptr;
    }
};

class Process : public Task 
{

protected:
    bool isCreated;
    
    Process *bigBrother;
    Process *smallBrother;
    Process *son;
    Process *father;
    
    

public:
    Process(VirtualMachine *vm, const char *name);
    ~Process();
    void set_parent(Process *p);
    void create() override;
    void update() override;
    void remove() override;

    void set_defaults();
    void set_variable(const String &name, Value value);
    Instance instance;
};

struct Hook
{
    void (* instance_create_hook)(Instance *);
    void (* instance_destroy_hook)(Instance *);
    void (* instance_pre_execute_hook)(Instance *);
    void (* instance_pos_execute_hook)(Instance *);
    void (* process_exec_hook)(Instance *);
};

void default_instance_create_hook(Instance *instance);
void default_instance_destroy_hook(Instance *instance);
void default_instance_pre_execute_hook(Instance *instance);
void default_instance_pos_execute_hook(Instance *instance);
void default_process_exec_hook(Instance *instance);

class VirtualMachine
{
    friend class Process;
    friend class Task;
    friend class Lexer;
    friend class Parser;
    friend class ScopeStack;

    Parser parser;

    Vector<Task *> taskes;
    HashTable<Task *> taskesMap;
    HashTable<NativeFunctionObject *> nativeFunctions;
    Vector<FunctionObject*> scriptFunctions;
    HashTable<FunctionObject *> functionsMap;


    Scope *global;

    static Value DEFAULT;

    Task *mainTask;
    Task *currentTask;
    

   

    bool panicMode;
    bool isHalt;
    bool isDone;

    FunctionObject* newFunction(const char *name);
    bool getFunction(const char *name,FunctionObject **func);



    Task *newTask(const char *name);
    Task *getCurrentTask();
    Task *getTask(const char *name);

    Task *addTask(const char *name);
    
    Process *AddProcess(const char *name);

    void setMainTask();
    void switchTask(Task *task);
    void setCurrentTask(Task *task);

    void Error(const char *format, ...);
    void Warning(const char *format, ...);
    void Info(const char *format, ...);

    int callNativeFunction(const char *name, Value *args, u8 argCount);

    u8 RunTask();



    bool isGlobalScope();

    List run_process;


    

public:
    VirtualMachine();
    ~VirtualMachine();
    
    void Clear();
    
    u8 Run();
    bool Compile(String source);
    bool IsReady();
    bool Update();


    Task *getMainTask();
    void disassemble();

    void registerFunction(const char *name, NativeFunction func, size_t arity);
    bool registerVariable(const char *name, Value value);
    bool registerNumber(const char *name, double value);
    bool registerInteger(const char *name, int value);
    bool registerString(const char *name, const char *value);
    bool registerBoolean(const char *name, bool value);
    bool registerNil(const char *name);
    bool ContainsVariable(const char *name);


    bool  push(Value v);
    Value pop();
    Value peek(int offset = 0);
    void  pop(u32 count);
    Value top();


    bool push_int(int value);
    bool push_double(double value);
    bool push_float(float value);
    bool push_long(long value);
    bool push_string(const char *str);
    bool push_string(const String &str);
    bool push_bool(bool value);
    bool push_nil();

    bool is_number();
    bool is_string();
    bool is_bool();
    bool is_nil();

    long   pop_int();
    double pop_double();
    float  pop_float();
    long   pop_long();
    String pop_string();
    bool   pop_bool();
    bool   pop_nil();

    Hook hooks;
};


