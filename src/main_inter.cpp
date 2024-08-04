
#include "pch.h"


#include "Config.hpp"
#include "Utils.hpp"
#include "Vm.hpp"








inline void PrintValue(Value value)
{
    if (IS_STRING(value))
        printf("%s\n", AS_STRING(value)->string.c_str());
    else if (IS_NUMBER(value))
        printf("%g\n", AS_NUMBER(value));
    else if (IS_BOOLEAN(value))
        printf("%s\n", AS_BOOLEAN(value) ? "true" : "false");
    else if (IS_NONE(value))
        printf("none");
}

static int native_write(VirtualMachine *vm, int argc, Value *args)
{
    for (int i = 0; i < argc; i++)
    {
        Value value = args[i];
        if (IS_STRING(value))
            printf("%s", AS_STRING(value)->string.c_str());
        else if (IS_NUMBER(value))
            printf("%g", AS_NUMBER(value));
        else if (IS_BOOLEAN(value))
            printf("%s", AS_BOOLEAN(value) ? "true" : "false");
        else if (IS_NONE(value))
            printf("none");
    }
    return 0;
}
static int native_writeln(VirtualMachine *vm, int argc, Value *args)
{
    for (int i = 0; i < argc; i++)
    {
        Value value = args[i];
        if (IS_STRING(value))
            printf("%s", AS_STRING(value)->string.c_str());
        else if (IS_NUMBER(value))
            printf("%g", AS_NUMBER(value));
        else if (IS_BOOLEAN(value))
            printf("%s", AS_BOOLEAN(value) ? "true" : "false");
        else if (IS_NONE(value))
            printf("none");
    }
    printf("\n");
    return 0;
}

static int native_clock(VirtualMachine *vm, int argc, Value *args)
{
    Value v = NUMBER((double)clock() / CLOCKS_PER_SEC);

    vm->push(std::move(v));

    return 1;
}

static int native_to_int(VirtualMachine *vm, int argc, Value *args)
{

    if (IS_NUMBER(args[0]))
    {
        int value = (int)AS_NUMBER(args[0]);
        double number = static_cast<double>(value);
        vm->push(NUMBER(number));
    }
    else if (IS_BOOLEAN(args[0]))
    {
        double number = AS_BOOLEAN(args[0]) ? 1 : 0;
        vm->push(NUMBER(number));
    }
    else if (IS_STRING(args[0]))
    {
        double number = atof(AS_STRING(args[0])->string.c_str());
        vm->push(NUMBER(number));
    }
    return 1;
}

static int native_to_string(VirtualMachine *vm, int argc, Value *args)
{
    if (IS_NUMBER(args[0]))
    {
        String value = String(AS_NUMBER(args[0]));
        vm->push(STRING(std::move(value)));
    }
    else if (IS_BOOLEAN(args[0]))
    {
        String value = String(AS_BOOLEAN(args[0]) ? "true" : "false");
        vm->push(STRING(std::move(value)));
    }
    else if (IS_STRING(args[0]))
    {
        const char *value = AS_RAW_STRING(args[0]);
        vm->push(STRING(value));
    }
    return 1;
}





static int native_rand(VirtualMachine *vm, int argc, Value *args)
{
    vm->push(NUMBER(Random()));
    return 1;
}






int main_inter()
{



    char *text = LoadTextFile("main.pc");
    if (!text)
    {
        ERROR("Failed to load main.pc");
        return 1;
    }
    String str(text);
    FreeTextFile(text);

    VirtualMachine vm;
    vm.registerFunction("write", native_write, -1);
    vm.registerFunction("writeln", native_writeln, -1);
    vm.registerFunction("clock", native_clock, 0);
    vm.registerFunction("toInt", native_to_int, 1);
    vm.registerFunction("toString", native_to_string, 1);
  


    vm.registerFunction("rand", native_rand, 0);



    bool sucess = vm.Compile(std::move(str));
    INFO("Compiled: %s", sucess ? "success" : "fail");




        if (!sucess)            return 0;

            vm.Run();

         if (sucess)
        {
            while(vm.size() > 0)
            {
             vm.Update();
            }
        }
        
        


    vm.Clear();

    

    return 0;
}