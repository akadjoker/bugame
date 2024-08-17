

#include "pch.h"
#include "Vm.hpp"
#include "Parser.hpp"

static u64 nextID = 0;

void debugValue(const Value &v)
{
    switch (v.type)
    {
    case ValueType::VSTRING:
        printf("%s", v.string->string.c_str());
        break;
    case ValueType::VNUMBER:
        printf("%g", v.number);
        break;
    case ValueType::VBOOLEAN:
        printf("%s", v.boolean ? "true" : "false");
        break;
    case ValueType::VNONE:
        printf("nil");
        break;

    default:
        printf("Unknow value ");
        break;
    }
}
void printValueln(const Value &v)
{
    switch (v.type)
    {
    case ValueType::VSTRING:
        printf("%s\n", v.string->string.c_str());
        break;
    case ValueType::VNUMBER:
        printf("%g\n", v.number);
        break;
    case ValueType::VBOOLEAN:
        printf("%s\n", v.boolean ? "true" : "false");
        break;
    case ValueType::VNONE:
        printf("nil\n");
        break;

    default:
        printf("Unknow value ");
        break;
    }
}

void printValue(const Value &v)
{
    switch (v.type)
    {
    case ValueType::VSTRING:
        PRINT("%s", v.string->string.c_str());
        break;
    case ValueType::VNUMBER:
        PRINT("%g", v.number);
        break;
    case ValueType::VBOOLEAN:
        PRINT("%s", v.boolean ? "true" : "false");
        break;
    case ValueType::VNONE:
        PRINT("nil");
        break;


    default:
        PRINT("Unknow value ");
        break;
    }
}

void Task::beginScope()
{
   // INFO("Begin scope %d", scopeDepth);
    scopeDepth++;
    
}

void Task::exitScope(int line)
{
    
    scopeDepth--;
  //  if (is_persistent) return;
    while (localCount > 0 && (locals[localCount - 1].depth > scopeDepth  && !locals[localCount - 1].isArg) )
    {
     //   INFO("End scope %d %d", scopeDepth,localCount);
        write_byte(OpCode::POP,line);
        localCount--;
    }
    
    
}
void *Task::operator new(size_t size)
{
  //  INFO("Allocate %d bytes", size);
    return Arena::as().allocate(size);
}

void Task::operator delete(void *ptr, size_t size)
{
    Arena::as().deallocate(ptr, size);
}

int Task::addLocal(const char *name, u32 len,bool isArg)
{
    if (localCount == UINT8_MAX)
    {
        vm->Error("Too many local variables in task");
        return -1;
    }
    Local *local = &locals[localCount++];
    strcpy(local->name, name);
    local->len = len;
    local->name[len] = '\0';
    local->depth = scopeDepth;
    local->isArg = isArg;
    if (isArg)
    {
      // constants.push_back(std::move(NONE()));
    }
    
  //  INFO("Add local %s in scope %d task %s", local->name, local->depth, this->name.c_str());
    

    return localCount - 1;
}
int Task::declareVariable(const String &string,bool isArg) 
{
    for (int i = localCount - 1; i >= 0; i--) 
    {
        Local* local = &locals[i];
        if (local->depth != -1 && local->depth < scopeDepth) 
        {
            break;
        }
        if (matchString(local->name, string.c_str(), string.length()))
        {
            vm->Error("Variable with this %s name already declared in this scope.", local->name);
            return -1;
        }
    }
    return  addLocal(string.c_str(),string.length(),isArg);
}

int Task::resolveLocal(const String &string) 
{
    for (int i = localCount - 1; i >= 0; i--) 
    {
        Local *local = &locals[i];
       // INFO("Resolve local %s in scope %d task %s", local->name, local->depth, this->name.c_str());
        if (matchString(local->name, string.c_str(), string.length()))
        {
            if (local->depth == -1) 
            {
                vm->Error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }
      return -1;
}
bool Task::setLocalVariable(const String &string, int index)
{
    if (index < 0 && index > UINT8_MAX  )
    {
        return false;
    }
    Local *local = &locals[index];
    strcpy(local->name, string.c_str());
    local->len = string.length();
    local->name[local->len] = '\0';
    local->depth = 0;
    localCount++;
    return true;

}
void Task::set_process()
{

 
    // addConstString("id");
    // addConstString("graph");
    // addConstString("x");
    // addConstString("y");
    // addConstNumber(0);
    // addConstNumber(0);
    // addConstNumber(0);
    // addConstNumber(0);
    


    setLocalVariable("id", IID);
    setLocalVariable("graph", IGRAPH);
    setLocalVariable("x", IX);
    setLocalVariable("y", IY);
 //   addConstString(name.c_str());
  //  setLocalVariable("type", ITYPE);

     
}
Value Task::top()
{
    return *stackTop;
}

bool Task::push(Value v)
{
    if (stackTop - stack >= STACK_MAX)
    {
        PrintStack();
        vm->Error("[PUSH] Stack overflow %s", name.c_str());
        return false;
    }
    *stackTop = std::move(v);
    stackTop++;
    return true;
}

Value Task::pop()
{
    if (stackTop == stack)
    {
       // INFO("[POP] is zero");
      //  return VirtualMachine::DEFAULT;
    }

    stackTop--;
    return std::move(*stackTop);
}

Value Task::peek(int distance)
{
    return stackTop[-1 - distance];
}

void Task::pop(u32 count)
{
    for (u32 i = 0; i < count; i++)
    {
        pop();
    }
}

Task::Task(VirtualMachine *vm, const char *name) 
{
    m_done = false;
    is_main = false;
    PanicMode = false;

 
    this->vm = vm;
    ID = nextID++;
    type = TaskType::TMAIN;
    parent = nullptr;
    argsCount = 0;
    isReturned = false;

    frame_counter = 0;
    last_frame_time = clock();

    state = RUNNING;
    frameStep = RUNNING;

    this->name = name;
    constants.reserve(256);
    frameDepth = 1024;

    scopeDepth = 0;
    localCount = 0;

    // lineBuffer = lines.pointer();
    

    stackTop = stack;
    chunk = nullptr;

    // INFO("Create task %s with id %d", name, ID);


}

Task::~Task()
{

   
    if (chunk!=nullptr)
    {
        delete chunk;
        chunk = nullptr;
    }
    
   //  INFO("Destroy task: %s with id %d", name.c_str(),constants.size());


     

    constants.clear();


    //INFO("Destroy task %s with id %d", name, ID);
}

void Task::init_frames()
{
    frameCount = 0;
    Frame *frame = &frames[frameCount++];
    frame->task = this;
    frame->slots = stack;
    frame->ip = chunk->code;
}

const char *opcodeNames[] = {
    "NONE",
    "PUSH",
    "POP",
    "CONST",
    "RETURN",
    "HALT",
    "PRINT",
    "NOW",
    "FRAME",

    "ADD",
    "SUBTRACT",
    "MULTIPLY",
    "DIVIDE",

    "MOD",
    "POWER",
    "NEGATE",

    "EQUAL",
    "NOT_EQUAL",
    "GREATER",
    "LESS",
    "GREATER_EQUAL",
    "LESS_EQUAL",

    "TRUE",
    "FALSE",
    "NOT",
    "AND",
    "OR",
    "XOR",
    "INC",
    "DEC",
    "SHL",
    "SHR",

    "ENTER_SCOPE",
    "EXIT_SCOPE",

    "GLOBAL_DEFINE",
    "GLOBAL_GET",
    "GLOBAL_ASSIGN",

    "LOCAL_DEFINE",
    "LOCAL_GET",
    "LOCAL_ASSIGN",

    "SWITCH",
    "CASE",
    "SWITCH_DEFAULT",

    "DUP",
    "EVAL_EQUAL",

    "JUMP_BACK",

    "LOOP_BEGIN",
    "LOOP_END",

    "BREAK",
    "CONTINUE",

    "DROP",
    "CALL",
    "CALL_SCRIPT",
    "CALL_PROCESS",
    "RETURN_DEF",
    "RETURN_PROCESS",
    "RETURN_NATIVE",

    "NIL",

    "JUMP",
    "JUMP_IF_FALSE",
    "JUMP_IF_TRUE",
    "COUNT"};

void Task::write_byte(u8 byte, int line)
{

    // if (byte < COUNT)
    // {
    //     printf("    write %d %d %s \n", chunk.count, line,opcodeNames[byte]);
    // }
    chunk->write(byte, line);
}



u8 Task::addConst(Value v)
{

    for (size_t i = 0; i < constants.size(); i++)
    {
        if (MatchValue(constants[i], v))
        {
           ///  INFO("recycle constant %d", i);
            return (u8)i;
        }
    }

    constants.push_back(std::move(v));
    if (constants.size() > 256)
    {
        vm->Error("Too many constants in  task");
        return 0;
    }
  
    return (u8)  constants.size() - 1;
}

u8 Task::addConstString(const char *str)
{
    return addConst(STRING(str));
}

u8 Task::addConstNumber(double number)
{
    return addConst(NUMBER(number));
}

void Task::writeByte(u8 byte, int line)
{
    write_byte(byte, line);
}

void Task::writeBytes(u8 byte1, u8 byte2, int line)
{
    writeByte(byte1, line);
    writeByte(byte2, line);
}
void Task::writeConstant(Value value, int line)
{
    writeBytes(OpCode::CONST, makeConstant(std::move(value)), line);
}
u8 Task::makeConstant(Value value)
{
    int constant = (int)addConst(std::move(value));
    if (constant > 255)
    {
        vm->Error("Too many constants in  task");
        return 0;
    }
    return (u8)constant;
}




void Task::disassembleCode(const char *name)
{

    printf("================== %s ==================\n", name);
    printf("\n");
    for (size_t offset = 0; offset < chunk->count;)
    {
        offset = disassembleInstruction(offset);
    }
    printf("\n");
}

u32 Task::disassembleInstruction(u32 offset)
{

    printf("%04d ", offset);
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", chunk->lines[offset]);
    }
    u8 instruction = chunk->code[offset];
    switch ((OpCode)instruction)
    {

    case OpCode::TRUE:
        return simpleInstruction("TRUE", offset);
    case OpCode::NIL:
        return simpleInstruction("NIL", offset);
    case OpCode::FALSE:
        return simpleInstruction("FALSE", offset);
    case OpCode::CONST:
        return constantInstruction("CONSTANT", offset);
    case OpCode::PROGRAM:
        return constantInstruction("PROGRAM", offset);
        
    case OpCode::POP:
        return simpleInstruction("POP", offset);
    case OpCode::PUSH:
        return simpleInstruction("PUSH", offset);
    case OpCode::RETURN:
        return simpleInstruction("RETURN", offset);
    case OpCode::HALT:
        return simpleInstruction("HALT", offset);
    case OpCode::PRINT:
        return simpleInstruction("PRINT", offset);
    case OpCode::NOW:
        return simpleInstruction("NOW", offset);
    case OpCode::FRAME:
        return simpleInstruction("FRAME", offset);
    case OpCode::CLONE:
        return simpleInstruction("CLONE", offset);
    case OpCode::ADD:
        return simpleInstruction("ADD", offset);
    case OpCode::SUBTRACT:
        return simpleInstruction("SUBTRACT", offset);
    case OpCode::MULTIPLY:
        return simpleInstruction("MULTIPLY", offset);
    case OpCode::DIVIDE:
        return simpleInstruction("DIVIDE", offset);
    case OpCode::MOD:
        return simpleInstruction("MOD", offset);
    case OpCode::POWER:
        return simpleInstruction("POWER", offset);
    case OpCode::NEGATE:
        return simpleInstruction("NEGATE", offset);
    case OpCode::EQUAL:
        return simpleInstruction("EQUAL", offset);
    case OpCode::NOT_EQUAL:
        return simpleInstruction("NOT_EQUAL", offset);
    case OpCode::GREATER:
        return simpleInstruction("GREATER", offset);
    case OpCode::GREATER_EQUAL:
        return simpleInstruction("GREATER_EQUAL", offset);
    case OpCode::LESS:
        return simpleInstruction("LESS", offset);
    case OpCode::LESS_EQUAL:
        return simpleInstruction("LESS_EQUAL", offset);
    case OpCode::NOT:
        return simpleInstruction("NOT", offset);
    case OpCode::XOR:
        return simpleInstruction("XOR", offset);
    case OpCode::AND:
        return simpleInstruction("AND", offset);
    case OpCode::OR:
        return simpleInstruction("OR", offset);
    case OpCode::SHL:
        return simpleInstruction("SHL", offset);
    case OpCode::SHR:
        return simpleInstruction("SHR", offset);

    case OpCode::LOOP_BEGIN:
        return simpleInstruction("LOOP_BEGIN", offset);
    case OpCode::LOOP_END:
        return simpleInstruction("LOOP_END", offset);

    case OpCode::BREAK:
        return simpleInstruction("BREAK", offset);
    case OpCode::CONTINUE:
        return simpleInstruction("CONTINUE", offset);

    case OpCode::DUP:
        return simpleInstruction("DUP", offset);
    case OpCode::EVAL_EQUAL:
        return simpleInstruction("EVAL_EQUAL", offset);

  
    case OpCode::GLOBAL_DEFINE:
        return varInstruction("GLOBAL_DEFINE", offset);
    case OpCode::GLOBAL_ASSIGN:
        return varInstruction("GLOBAL_ASSIGN", offset);
    case OpCode::GLOBAL_GET:
        return varInstruction("GLOBAL_GET", offset);

    case OpCode::LOCAL_SET:
        return byteInstruction("LOCAL_SET", offset);
    case OpCode::LOCAL_GET:
        return byteInstruction("LOCAL_GET", offset);

    case OpCode::SWITCH:
        return simpleInstruction("SWITCH", offset);
    case OpCode::CASE:
        return jumpInstruction("CASE", 1, offset);
    case OpCode::SWITCH_DEFAULT:
        return jumpInstruction("DEFAULT", 1, offset);

    case OpCode::JUMP_BACK:
        return jumpInstruction("JUMP_BACK", -1, offset);

    case OpCode::JUMP:
        return jumpInstruction("JUMP", 1, offset);
    case OpCode::JUMP_IF_FALSE:
        return jumpInstruction("JUMP_IF_FALSE", 1, offset);
    case OpCode::JUMP_IF_TRUE:
        return jumpInstruction("JUMP_IF_TRUE", 1, offset);

    case OpCode::CALL:
        return byteInstruction("CALL_NATIVE", offset);
    case OpCode::CALL_SCRIPT:
        return byteInstruction("CALL_SCRIPT", offset);
    case OpCode::CALL_PROCESS:
        return byteInstruction("CALL_PROCESS", offset);

    case OpCode::RETURN_DEF:
        return byteInstruction("DEF_RETURN", offset);
    case OpCode::RETURN_PROCESS:
        return simpleInstruction("PROCESS_RETURN", offset);

    default:
    {
        printf("Unknow instruction %d\n", instruction);
        return chunk->count;
        break;
    }
    }
    return offset;
}

u32 Task::byteInstruction(const char *name, u32 offset)
{
    u8 slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

u32 Task::jumpInstruction(const char *name, u32 sign, u32 offset)
{
    u16 jump = (u16)chunk->code[offset + 1] << 8;
    jump |= chunk->code[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

u32 Task::varInstruction(const char *name, u32 offset)
{
    u8 constant = chunk->code[offset + 1];

    printf("%-16s %4d '", name, constant);
    Value value = constants[constant];
    debugValue(value);
    printf("'\n");

    return offset + 2;
}



u32 Task::constantInstruction(const char *name, u32 offset)
{

    u8 constant = chunk->code[offset + 1];

    printf("%-16s %4d '", name, constant);
    Value value = constants[constant];

    debugValue(value);
    printf("'\n");

    return offset + 2;
}

u32 Task::simpleInstruction(const char *name, u32 offset)
{
    printf("%s\n", name);
    return offset + 1;
}

void Task::PrintStack()
{
    printf("          ");
    for (Value *slot = stack; slot < stackTop; slot++)
    {
        printf("[ ");
        debugValue(*slot);
        printf("] ");
    }

    printf("\n");
}

bool Task::IsDone()
{
    return m_done;
}

void Task::Done()
{
    m_done = true;
}

//***************************************************************************************************************** */
//***************************************************************************************************************** */
//***************************************************************************************************************** */
static const size_t instructionsPerFrame = 30;
static const u16 fps = 60;


u8 Task::Pause()
{
     if (state == PAUSED) 
     {
        clock_t now = clock();
        double elapsed = (double)(now - last_frame_time) / CLOCKS_PER_SEC * 1000;
        double frame_duration = 1000 / fps;


      //  printf("Elapsed: %f Frame duration: %f\n", elapsed, frame_duration);
        
        if (elapsed >= frame_counter * frame_duration) 
        {
            state = RUNNING;
            frame_counter = 0;
            last_frame_time = now;
           // return RUNNING;
        } else 
        {
            return PAUSED;
        }
    
    }

    return RUNNING;
}

u8 Task::Run()
{
    
    if (PanicMode)         return ABORTED;
    if (isReturned)        return FINISHED;





 Frame* frame = &frames[frameCount - 1];        

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
    (frame->ip += 2,        \
     (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() (frame->task->constants[READ_BYTE()])

    u32 instructionsExecuted=0;

    
     if (state == PAUSED) 
     {
        clock_t now = clock();
        double elapsed = (double)(now - last_frame_time) / CLOCKS_PER_SEC * 1000;
        double frame_duration = 1000 / fps;


      //  printf("%s Elapsed: %f Frame duration: %f total: %f  %d\n",name.c_str(), elapsed, frame_duration,(frame_counter * frame_duration),*frame->ip);
        
        if (elapsed >= frame_counter * frame_duration) 
        {
            state = RUNNING;
            frame_counter = 0;
            last_frame_time = now;
        } else 
        {
           state = PAUSED;
           return PAUSED;
        }
     } 



     while (instructionsExecuted < instructionsPerFrame)
   // for (;;)
    {

        u8 instruction = READ_BYTE();
        int line = frame->task->chunk->lines[instruction];

        switch ((OpCode)instruction)
        {
        case OpCode::CONST:
        {

            Value value = READ_CONSTANT();
            push(value);

            break;
        }
        case OpCode::PUSH:
        {

            Value value = READ_CONSTANT();
            push(std::move(value));
            break;
        }
        case OpCode::POP:
        {
            pop();

             
            break;
         }
         case OpCode::TRUE:
         {
             push(std::move(BOOLEAN(true)));
             break;
         }
         case OpCode::FALSE:
         {
             push(std::move(BOOLEAN(false)));
             break;
         }

         case OpCode::HALT:
         {
             vm->Warning("Halt!");
             isReturned = true;
             state = ABORTED;
             return state;
         }
         case OpCode::PROGRAM:
         {
             Value constant = READ_CONSTANT();
             if (!IS_STRING(constant))
             {
                 vm->Error("Program  name must be string [line %d]", line);
                state = ABORTED;
                    return state;
             }
             break;
         }
         case OpCode::PRINT:
         {
             Value value = pop();
             // printValue(std::move(value));
             debugValue(std::move(value));
             printf("\n");
             break;
         }
         case OpCode::NOW:
         {
             push(std::move(NUMBER(time_now())));
             break;
         }
         case OpCode::ADD:
         {
             u8 result = op_add();
             if (result != OK)
                 return result;
             break;
         }
         case OpCode::SUBTRACT:
         {
             Value b = pop();
             Value a = pop();
             if (IS_NUMBER(a) && IS_NUMBER(b))
             {
                 Value result = NUMBER(AS_NUMBER(a) - AS_NUMBER(b));
                 push(std::move(result));
             }
             else
             {
                 vm->Error("invalid  'subtract' operands [line %d]", line);

             state = ABORTED;
             return state;
             }

             break;
         }
         case OpCode::MULTIPLY:
         {
             Value b = pop();
             Value a = pop();
             if (IS_NUMBER(a) && IS_NUMBER(b))
             {
                 Value result = NUMBER(AS_NUMBER(a) * AS_NUMBER(b));
                 push(std::move(result));
             }
             else
             {
                 vm->Error("invalid 'multiply' operands [line %d]", line);
                 return ABORTED;
             }
             break;
         }
         case OpCode::DIVIDE:
         {
             Value b = pop();
             Value a = pop();
             if (IS_NUMBER(a) && IS_NUMBER(b))
             {
                 if (AS_NUMBER(b) == 0)
                 {
                     vm->Error("division by zero [line %d]", line);

                     return ABORTED;
                 }
                 Value result = NUMBER(AS_NUMBER(a) / AS_NUMBER(b));
                 push(std::move(result));
             }
             else
             {
                 vm->Error("invalid 'divide' operands [line %d]", line);

                 return ABORTED;
             }
             break;
         }
         case OpCode::MOD:
         {
             u8 result = op_mod(line);
             if (result != OK)
                 return result;
             break;
         }
         case OpCode::POWER:
         {
             Value b = pop();
             Value a = pop();
             if (IS_NUMBER(a) && IS_NUMBER(b))
             {
                 Value result = NUMBER(pow(AS_NUMBER(a), AS_NUMBER(b)));
                 push(std::move(result));
             }
             else
             {
                 vm->Error("invalid 'power' operands [line %d]", line);

                 return ABORTED;
             }
             break;
         }
         case OpCode::NEGATE:
         {
             Value value = pop();
             if (IS_NUMBER(value))
             {
                 Value result = NUMBER(-AS_NUMBER(value));
                 push(std::move(result));
             }
             else
             {
                 vm->Error("invalid 'negate' operands, Operand must be a number.");

                 return ABORTED;
             }
             break;
         }
         case OpCode::NOT:
         {
             push(std::move(BOOLEAN(isFalsey(pop()))));
             break;
         }
         case OpCode::EQUAL:
         {
             Value b = pop();
             Value a = pop();
             bool result = MatchValue(a, b);
             push(std::move(BOOLEAN(result)));

             break;
         }
         case OpCode::EVAL_EQUAL:
         {
             Value b = peek(0);
             Value a = peek(1);
             bool result = MatchValue(a, b);
             push(std::move(BOOLEAN(result)));
             break;
         }
         case OpCode::NOT_EQUAL:
         {
             u8 result = op_not_equal();
             if (result != OK)
                 return result;
             break;
         }
         case OpCode::LESS:
         {
             u8 result = op_less();
             if (result != OK)
                 return result;

             break;
         }
         case OpCode::LESS_EQUAL:
         {
             u8 result = op_less_equal();
             if (result != OK)
                 return result;

             break;
         }
         case OpCode::GREATER:
         {
             u8 result = op_greater();
             if (result != OK)
                 return result;
             break;
         }
         case OpCode::GREATER_EQUAL:
         {
             u8 result = op_greater_equal();
             if (result != OK)
                 return result;
             break;
         }

         case OpCode::XOR:
         {
             u8 result = op_xor();
             if (result != OK)
                 return result;
             break;
         }

         case OpCode::GLOBAL_DEFINE:
         {

             Value constant = READ_CONSTANT();
             if (!IS_STRING(constant))
             {
                 vm->Error("Variable  name must be string [line %d]", line);

                 return ABORTED;
             }
             const char *name = AS_RAW_STRING(constant);
             Value value = peek();

             if (!vm->global->define(name, value))
             {
                 vm->Error("Already a global variable with '%s' name.", name);
                 return ABORTED;
             }
             pop();
             //  printValue(value);

             break;
         }

         case OpCode::GLOBAL_ASSIGN:
         {
             Value constant = READ_CONSTANT();
             if (!IS_STRING(constant))
             {
                 vm->Error("Variable  name must be string [line %d]", line);

                 return ABORTED;
             }
             const char *name = AS_RAW_STRING(constant);
             Value value = peek();

             if (!vm->global->assign(name, value))
             {
                 vm->Warning("Undefined global variable '%s' [line %d]", name, line);
             }

             break;
         }
         case OpCode::GLOBAL_GET:
         {
             Value constant = READ_CONSTANT();
             if (!IS_STRING(constant))
             {
                 vm->Error("Variable  names must be string [line %d]", line);

                 return ABORTED;
             }
             const char *name = AS_RAW_STRING(constant);
             Value value;

             if (vm->global->get(name, value))
             {
                 push(value);
             }
             else
             {
                 vm->Error("Undefined global variable '%s' [line %d]", name, line);

                 return ABORTED;
             }

             break;
         }
         case OpCode::LOCAL_SET:
         {
             u8 slot = READ_BYTE();

             if (type == TaskType::TPROCESS)
             {
                 if (slot == IID)
                 {
                     vm->Error("Variable  ID is read-only");
                     return ABORTED;
                 }
                 
             }

             frame->slots[slot] = peek(0);

            //   printf("local set variable %d", slot);
          //     printValue(frame->slots[slot]);

             break;
         }
         case OpCode::LOCAL_GET:
         {
             u8 slot = READ_BYTE();
           

           //  INFO("local get variable %d ", slot);
           //  printValue(frame->slots[slot]);
            
             push(frame->slots[slot]);

             break;
         }

         case OpCode::JUMP_IF_FALSE:
         {
             u16 offset = READ_SHORT();
             Value value = peek(0);
             if (isFalsey(value))
             {
                 frame->ip += offset;
                 //   printf("offset: %d\n", offset);
             }
             break;
         }
         case OpCode::JUMP_IF_TRUE:
         {
             u16 offset = READ_SHORT();
             frame->ip += offset;
             break;
         }
         case OpCode::JUMP:
         {
             u16 offset = READ_SHORT();
             frame->ip += offset;
             //  printf("jump offset: %d\n", offset);
             break;
         }
         case OpCode::DUP:
         {
             Value value = peek(0);
             push(value);
             break;
         }
         case OpCode::JUMP_BACK:
         {
             uint16_t offset = READ_SHORT();
             frame->ip -= offset;
             break;
         }

         case OpCode::CALL:
         {
             uint8_t argCount = READ_BYTE();
             Value func = peek(argCount);
             if (!IS_STRING(func))
             {
                 vm->Error("Native Function name must be string [line %d]", line);
                 printValue(func);
                 return ABORTED;
             }
             Value result;
             int popResult = -1;
             vm->currentTask = this;
             int count = vm->callNativeFunction(AS_RAW_STRING(func), (stackTop - argCount), argCount);
             if (count == -1)
             {
                 return ABORTED;
             }
             if (count > 0)
             {
                 popResult = count;
                 result = peek();
             }
             stackTop -= (argCount + 1) + popResult;
             if (count > 0)
                 push(std::move(result));
            
             break;
         }
         case OpCode::CALL_SCRIPT:
         {

             int argCount = (int)READ_BYTE();
             Value func = peek(argCount);
             if (!IS_STRING(func))
             {
                 vm->Error("Function name must be string [line %d]", line);
                 printValue(func);
                 return ABORTED;
             }
             FunctionObject *callTask = nullptr;
             if (!vm->getFunction(AS_RAW_STRING(func), &callTask))
             {
                 vm->Error("Function '%s' not defined [line %d]", AS_RAW_STRING(func), line);
                 return ABORTED;
             }
             if (callTask->argsCount != argCount)
             {
                 vm->Error("Function %s, Expected %d arguments but got %d [line %d]", AS_RAW_STRING(func), callTask->argsCount, argCount, line);
                 return ABORTED;
             }

             frame = &frames[frameCount++];
             frame->task = callTask;
             frame->ip = callTask->chunk->code;
             frame->slots = stackTop - argCount - 1;

             callTask->disassembleCode(callTask->name.c_str());

             if (frameCount == MAX_FRAMES)
             {
                 vm->Error("Frames  overflow .");
                 return ABORTED;
             }
             break;
         }
         case OpCode::RETURN:
         {

             // PrintStack();
             Value result = pop();
             frameCount--;
             if (frameCount == 0)
             {
                 isReturned = true;
                 INFO("main %s", frame->task->name.c_str());
                 PrintStack();
                 pop();
                 state = TERMINATED;
                 return TERMINATED;
             }
             //  INFO("return %s", frame->task->name.c_str());
             frame->task->exitScope(line);
             stackTop = frame->slots;
             push(result);
             frame = &frames[frameCount - 1];

             break;
         }
         case OpCode::CALL_PROCESS:
         {

             u8 argCount = READ_BYTE();
             Value func = peek(argCount);
             if (!IS_STRING(func))
             {
                 vm->Warning("Process name must be a string");
                 printValue(func);

                 return ABORTED;
             }
             const char *name = AS_RAW_STRING(func);
             Task *callTask = vm->getTask(AS_RAW_STRING(func));
             if (!callTask)
             {
                 pop(argCount);
                 vm->Error("Process '%s' not defined [line %d]", AS_RAW_STRING(func), line);

                 return ABORTED;
             }
             if (callTask->argsCount != argCount)
             {
                 vm->Error("Process %s, Expected %ld arguments but got %ld [line %d]", AS_RAW_STRING(func), callTask->argsCount, argCount, line);

                 return ABORTED;
             }

             // INFO("Process CALL %s args %d chunk %d", name ,callTask->argsCount,callTask->chunk->count);
             Process *process = vm->AddProcess(name);
             process->chunk = new Chunk(callTask->chunk);
            
            
             process->push(INTEGER(process->ID));
             process->push(INTEGER(100));
             process->push(NUMBER(2));
             process->push(NUMBER(3));
             process->push(NUMBER(3));

      
            
             process->init_frames();  // prepare frames 4 functions
             process->frames[0].slots = process->stackTop - 4  -1 ;




            
            // process->set_defaults(); // local variables x,y, ... etc
             process->constants = callTask->constants;
          //   process->constants.push_back(INTEGER(process->ID));
            //  process->addConstNumber(0);
            //  process->addConstNumber(1);
            //  process->addConstNumber(2);
            //  process->addConstNumber(3);
           
            // for (u32 i = 0; i < callTask->constants.size(); i++)
            // {
            //     process->constants.push_back(callTask->constants[i]);
            // }
            //  process->constants.push_back(STRING("pocess"));

            // for (u32 i = 0; i < callTask->constants.size(); i++)
            // {
            //    // process->constants[i+1]=callTask->constants[i];
            // }
          //  process->constants.push_back(STRING("pocess"));
       


          //  process->push(NUMBER(3));


             for (int i = argCount - 1; i >= 0; i--)
             {
                 Value arg = peek(i);
                 process->push(arg);
             }
             pop(argCount+1);
           

            


             frame->slots = stackTop - argCount - 1;
               callTask->disassembleCode(name);
             //   process->disassembleCode(name);
             if (this->type == TaskType::TPROCESS)
             {
                 process->set_parent(static_cast<Process *>(this));
             }

            // vm->run_process.push_back(process);
             vm->processList.add(process);
             push(INTEGER((int)process->ID));
             process->set_defaults(); // local variables x,y, ... etc

            // process->render();

             // PrintStack();
             return RUNNING;
         }
         case OpCode::RETURN_PROCESS:
         {
             isReturned = true;
             // disassembleCode(name.c_str());
             // INFO("Process RETURN %s ", name.c_str());
             // pop((constants.size() - 1) + DEFAULT_COUNT);
             int count = (stackTop - frame->slots);
             for (int i = 0; i < count; i++)
             {
                 pop();
             }

             // PrintStack();
             state = TERMINATED;
             return TERMINATED;
         }

         case OpCode::FRAME:
         {
                Value constant = pop();
                double frame_value = AS_NUMBER(constant);

                if (frame_value > 1) frame_value = 1;
                if (frame_value < 0) frame_value = 0;

                frame_counter = 0.01;
                last_frame_time = clock();
                Process *process = static_cast<Process *>(frame->task);
                process->create();


                state = PAUSED;
                return state;

         }
         case OpCode::CLONE:
         {
             INFO("CLONE %s", name.c_str());
             break;
         }

         case OpCode::NIL:
         {
             push(VirtualMachine::DEFAULT);
             break;
         }

         default:
         {
             vm->Error(" %s running %d with unknown '%d' opcode frame %d", name.c_str(), frame->ip, (int)instruction, frameCount);
             state = ABORTED;
             return ABORTED;
         }
         }

           //  INFO("RUN %s executed %d", name.c_str(),instructionsExecuted);

         instructionsExecuted++;
         if (instructionsExecuted >= instructionsPerFrame)
         {
             state = RUNNING;
             return RUNNING;
         }
       // state = RUNNING;
      //  return state;

     }


return FINISHED;

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT

}