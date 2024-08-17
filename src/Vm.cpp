
#include "pch.h"
#include "Vm.hpp"

extern void printValue(const Value &v);
extern void debugValue(const Value &v);
extern void printValueln(const Value &v);

Value VirtualMachine::DEFAULT = NONE();

Scope::Scope()
{
    level = 0;
}

Scope::Scope( int level)
{
    this->level = level;
    // INFO("New scope: %s %d", name, level);
}

Scope::~Scope()
{
    // INFO("Delete scope: %s", name.c_str());
}

bool Scope::define(const char *name, Value value)
{
    if (variables.contains(name))
    {
        return false;
    }
    variables.insert(name, std::move(value));
    return true;
}

void Scope::insert(const char *name, Value value)
{
    variables.insert(name, std::move(value));
}

bool Scope::get(const char *name, Value &value)
{
    return variables.find(name, value);
}

bool Scope::contains(const char *name) const
{
    return variables.contains(name);
}

bool Scope::assign(const char *name, Value value)
{
    return variables.change(name, std::move(value));
}



void Scope::print()
{
    
}

void Scope::clear()
{
    // INFO("Clear scope: %s", name.c_str());
    variables.clear();
}

Task *VirtualMachine::getCurrentTask()
{
    return currentTask;
}

Task *VirtualMachine::getTask(const char *name)
{
    Task *task;
    if (taskesMap.find(name, task))
    {
        return task;
    }
    return nullptr;
}

FunctionObject *VirtualMachine::newFunction(const char *name)
{
    FunctionObject *task = new FunctionObject(this, name);
    functionsMap.insert(name, task);
    currentTask = task;
    return task;
}

bool VirtualMachine::getFunction(const char *name, FunctionObject **func)
{
    return functionsMap.find(name, *func);
}

Task *VirtualMachine::newTask(const char *name)
{
    Task *task = new Task(this, name);
    currentTask = task;
    taskes.push_back(task);
    taskesMap.insert(name, task);
    return task;
}

Task *VirtualMachine::addTask(const char *name)//functions task ,small chunk
{
    Task *task = new Task(this, name);
    taskes.push_back(task);
    return task;
}

Process *VirtualMachine::AddProcess(const char *name)
{
    Process *task = new Process(this, name);
    return task;
}

void VirtualMachine::setMainTask()
{
    if (currentTask != mainTask)
    {
        // currentTask->disassembleCode(currentTask->name.c_str());
    }

    currentTask = mainTask;
}

void VirtualMachine::switchTask(Task *task)
{

    task->parent = currentTask;

    // INFO("switch task %s from %s",task->name.c_str(),task->parent->name.c_str());
    currentTask = task;
}

void VirtualMachine::setCurrentTask(Task *task)
{
    currentTask = task;
}

void VirtualMachine::Error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Log(2, format, args);
    va_end(args);
    panicMode = true;
}

void VirtualMachine::Warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Log(1, format, args);
    va_end(args);
}

void VirtualMachine::Info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Log(0, format, args);
    va_end(args);
}

int VirtualMachine::callNativeFunction(const char *name, Value *args, u8 argCount)
{
    NativeFunctionObject *func;
    if (nativeFunctions.find(name, func))
    {
        if (func->arity != -1)
        {
            if (func->arity != argCount)
            {
                Error("Native function %s has wrong number of arguments. Expected %d but got %d", name, func->arity, argCount);
                return -1;
            }
        }
        return func->call(this, argCount, args);
    }
    Error("Native function %s not found", name);
    return -1;
}

Task *VirtualMachine::getMainTask()
{
    return mainTask;
}

void VirtualMachine::disassemble()
{
    PRINT("Disassembly tasks");
}

void VirtualMachine::registerFunction(const char *name, NativeFunction func, size_t arity)
{
    if (nativeFunctions.contains(name))
    {
        Warning("Function %s already exists", name);
        return;
    }
    parser.addNative(name);
    nativeFunctions.insert(name, new NativeFunctionObject(func, name, arity));
}

bool VirtualMachine::registerVariable(const char *name, Value value)
{
    return global->define(name, std::move(value));
}

bool VirtualMachine::registerNumber(const char *name, double value)
{
    return global->define(name, std::move(NUMBER(value)));
}

bool VirtualMachine::registerInteger(const char *name, int value)
{
    return global->define(name, std::move(INTEGER(value)));
}

bool VirtualMachine::registerString(const char *name, const char *value)
{
    return global->define(name, std::move(STRING(value)));
}

bool VirtualMachine::registerBoolean(const char *name, bool value)
{
    return global->define(name, std::move(BOOLEAN(value)));
}

bool VirtualMachine::ContainsVariable(const char *name)
{
    return global->contains(name);
}

VirtualMachine::VirtualMachine()
{
    hooks.instance_create_hook = default_instance_create_hook;
    hooks.instance_destroy_hook = default_instance_destroy_hook;
    hooks.instance_pos_execute_hook = default_instance_pos_execute_hook;
    hooks.instance_pre_execute_hook = default_instance_pre_execute_hook;
    hooks.process_exec_hook = default_process_exec_hook;

    global = new Scope(0);
    mainTask = new Task(this, "__main__");
    mainTask->chunk = new Chunk(1025);
    mainTask->is_main = true;
    currentTask = mainTask;

    
    taskes.push_back(mainTask);
    taskesMap.insert("__main__", mainTask);

    panicMode = false;
    isHalt = false;
    isDone = false;
    parser.Init(this);
}

bool VirtualMachine::Compile(String source)
{
    if (parser.Load(std::move(source)))
    {
        return parser.Process();
    }
    return false;
}

bool VirtualMachine::IsReady()
{
    return !panicMode && !isHalt;
}

bool VirtualMachine::Update()
{
    if (panicMode || isHalt )
        return false;

    Process *task = processList.head;
    while (task)
    {
        Process *next = task->next;

        u8 state = task->Run();
        if (state == ABORTED || state == TERMINATED || state == FINISHED)
        {
            if (processList.remove(task))
            {
                task->remove();
                task->next = nullptr;
                task->prev = nullptr;
                cleaner.add(task);
            }
         
        }
        if (state ==PAUSED)
        {
           // state = task->Pause();
        // task->update();
         //   printf("Paused\n");

        }else   if (state == RUNNING)
        {
           
           // task->render();

          //  printf("Running\n");
        }
         task->render();

       

        task = next;
    }


  //  if (cleaner.count() > 256)
        cleaner.clear(true);

    Arena::as().gc();
    return processList.count() == 0;
}

VirtualMachine::~VirtualMachine()
{
    Arena::as().clear();
    Clear();
    delete global;
}

void VirtualMachine::Clear()
{

   // scriptFunctions.clear();
    FunctionObject* func = functionsMap.first();
    while (func)
    {
        delete func;
        func = functionsMap.next();
    }

    functionsMap.clear();
    for (size_t i = 0; i < taskes.size(); i++)
    {
        delete taskes[i];
    }
    taskes.clear();
    taskesMap.clear();


    NativeFunctionObject* nativeOP = nativeFunctions.first();
    while (nativeOP)
    {
        delete nativeOP;
        nativeOP = nativeFunctions.next();
    }
    nativeFunctions.clear();

    cleaner.clear(true);
    processList.clear(true);

    global->clear();
    mainTask = nullptr;
    currentTask = nullptr;
}

bool VirtualMachine::push(Value v)
{
    return currentTask->push(std::move(v));
}

Value VirtualMachine::pop()
{
    return currentTask->pop();
}

Value VirtualMachine::peek(int offset)
{
    return currentTask->peek(offset);
}

Value VirtualMachine::top()
{
    return currentTask->top();
}

long VirtualMachine::pop_int()
{
    Value value = pop();
    if (IS_NUMBER(value))
    {
        double number = AS_NUMBER(value);
        return (long)number;
    }
    Error("Expected number but got :");
    printValue(value);
    return 0;
}

double VirtualMachine::pop_double()
{
    Value value = pop();
    if (IS_NUMBER(value))
    {
        double number = AS_NUMBER(value);
        return number;
    }
    Error("Expected number but got :");
    printValue(value);
    return 0;
}

float VirtualMachine::pop_float()
{
    Value value = pop();
    if (IS_NUMBER(value))
    {
        double number = AS_NUMBER(value);
        return (float)number;
    }
    Error("Expected number but got :");
    printValue(value);
    return 0;
}

long VirtualMachine::pop_long()
{
    Value value = pop();
    if (IS_NUMBER(value))
    {
        double number = AS_NUMBER(value);
        return (long)number;
    }
    Error("Expected number but got :");
    printValue(value);
    return 0;
}

String VirtualMachine::pop_string()
{
    Value value = pop();
    if (IS_STRING(value))
    {
        String str = AS_STRING(value)->string;
        return str;
    }
    Error("Expected string but got :");
    printValue(value);
    return "";
}

bool VirtualMachine::pop_bool()
{
    Value value = pop();
    if (IS_BOOLEAN(value))
    {
        return AS_BOOLEAN(value);
    }
    Error("Expected bool but got :");
    printValue(value);
    return false;
}

bool VirtualMachine::pop_nil()
{
    Value value = pop();
    if (IS_NONE(value))
    {
        return true;
    }
    Error("Expected nil but got :");
    printValue(value);
    return false;
}

bool VirtualMachine::push_int(int value)
{
    double number = static_cast<double>(value);
    return push(std::move(NUMBER(number)));
}

bool VirtualMachine::push_double(double value)
{
    return push(std::move(NUMBER(value)));
}

bool VirtualMachine::push_bool(bool value)
{
    return push(std::move(BOOLEAN(value)));
}

bool VirtualMachine::push_nil()
{
    return push(std::move(NONE()));
}

bool VirtualMachine::push_string(const char *value)
{
    return push(std::move(STRING(value)));
}

bool VirtualMachine::push_string(const String &str)
{
    return push(std::move(STRING(str.c_str())));
}

const String OpCodeNames[] =
    {
        "NONE", "PUSH", "POP", "CONST", "RETURN", "HALT", "PRINT", "NOW", "FRAME",
        "ADD", "SUBTRACT", "MULTIPLY", "DIVIDE", "MOD", "POWER", "NEGATE",
        "EQUAL", "NOT_EQUAL", "GREATER", "LESS", "GREATER_EQUAL", "LESS_EQUAL",
        "NOT", "AND", "OR", "XOR", "INC", "DEC", "SHL", "SHR", "ENTER_SCOPE", "EXIT_SCOPE",
        "GLOBAL_DEFINE", "GLOBAL_GET", "GLOBAL_ASSIGN", "LOCAL_DEFINE", "LOCAL_GET", "LOCAL_ASSIGN",
        "SWITCH", "CASE", "SWITCH_DEFAULT", "DUP", "EVAL_EQUAL", "JUMP_BACK", "LOOP_BEGIN", "LOOP_END",
        "BREAK", "CONTINUE", "DROP", "CALL", "CALL_SCRIPT", "CALL_PROCESS", "RETURN_DEF", "RETURN_PROCESS",
        "RETURN_NATIVE", "NIL", "JUMP", "JUMP_IF_FALSE", "JUMP_IF_TRUE", "COUNT"};
bool VirtualMachine::Run()
{
     mainTask->init_frames();
  //   mainTask->disassembleCode("main");
    while(true)
    {
        u8 state =mainTask->Run();
        if (state == FINISHED || state == TERMINATED)
        {
            return true;
        } 
        if (state == ABORTED)
        {
            return false;
        }
    }
    return false;
}

//***************************************************************************************************************** */
//***************************************************************************************************************** */
//***************************************************************************************************************** */

FunctionObject::FunctionObject(VirtualMachine *vm, const char *name) : Task(vm, name), arity(0)
{
    chunk = new Chunk(256);
}

//***************************************************************************************************************** */
//***************************************************************************************************************** */
//***************************************************************************************************************** */


NativeFunctionObject::NativeFunctionObject(NativeFunction func, const char *name, int arity) : func(func), name(name), arity(arity)
{
}

int NativeFunctionObject::call(VirtualMachine *vm, int argc, Value *args)
{
    return func(vm, argc, args);
}

//***************************************************************************************************************** */
//***************************************************************************************************************** */
//***************************************************************************************************************** */
