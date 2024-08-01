
#include "pch.h"

#include "Config.hpp"
#include "Vm.hpp"

extern void printValue(const Value &v);
extern void debugValue(const Value &v);
extern void printValueln(const Value &v);

void Process::set_parent(Process *p)
{
    father = p;
    if (p)
    {
        // Se o novo pai já tiver um filho, ajustamos os irmãos
        if (p->son)
        {
            // O filho existente se torna o irmão mais velho
            p->son->bigBrother = this;
            // Este processo se torna o irmão mais novo
            this->smallBrother = p->son;
        }
        
        // Este processo se torna o filho do pai
        p->son = this;
    }
}

Process::Process(VirtualMachine *vm, const char *name) : Task(vm, name)
{
    type = ObjectType::OPROCESS;

    isCreated = false;
  
    instance.ID = ID;
    instance.name = name;
    instance.user_data = nullptr;

    bigBrother=nullptr;
    smallBrother= nullptr;
    son = nullptr;
    father = nullptr;
    


    mark();
   // INFO("Create process: %s", name);

}

Process::~Process()
{
 // INFO("Destroy process: %s", name.c_str());

}

void Process::create()
{
    if (isCreated) return;
    // local->read("x", x);
    // local->read("y", y);
    // local->read("graph", graph);


    // instance.locals[IX]     = static_cast<s32>(AS_NUMBER(x));
    // instance.locals[IY]     = static_cast<s32>(AS_NUMBER(y));
    // instance.locals[IGRAPH] = static_cast<s32>(AS_NUMBER(graph));

    


    isCreated = true;
  
    vm->hooks.instance_create_hook(&instance);
    
}

void Process::update()
{
    if (!isCreated) 
    {
       
    }
    // local->read("x", x);
    // local->read("y", y);
    // local->read("graph", graph);


    // instance.locals[IX]     = static_cast<s32>(AS_NUMBER(x));
    // instance.locals[IY]     = static_cast<s32>(AS_NUMBER(y));
    // instance.locals[IGRAPH] = static_cast<s32>(AS_NUMBER(graph));
    vm->hooks.instance_pre_execute_hook(&instance);





   //printf("Process: %d\n", frames);
   if ( !isCreated)
   {
        create();
        return;
   }

    if (!isCreated) return;


    




    vm->hooks.process_exec_hook(&instance);




    // x = NUMBER(static_cast<double>(instance.locals[IX])); 
    // y = NUMBER(static_cast<double>(instance.locals[IY]));
    // graph = NUMBER(static_cast<double>(instance.locals[IGRAPH]));

    // local->write("x", std::move(x));
    // local->write("y", std::move(y));
    // local->write("graph", std::move(graph));


    vm->hooks.instance_pos_execute_hook(&instance);
   
   
}

void Process::remove()
{
    Done();
    unmark();

    // instance.locals[IX]     = static_cast<s32>(AS_NUMBER(x));
    // instance.locals[IY]     = static_cast<s32>(AS_NUMBER(y));
    // instance.locals[IGRAPH] = static_cast<s32>(AS_NUMBER(graph));
    vm->hooks.instance_destroy_hook(&instance);
}

void Process::set_defaults()
{
    set_process();
    frames[0].slots[IX] = std::move(NUMBER(0.1));
    frames[0].slots[IY] = std::move(NUMBER(0.2));
    frames[0].slots[IGRAPH] = std::move(NUMBER(0.3));
    frames[0].slots[IID] = std::move(NUMBER(0.4));
}

void Process::set_variable(const String &name, Value value)
{
    if (matchString("x",name.c_str(), name.length()))
    {
        frames[0].slots[IX] = std::move(value);
    } else if (matchString("y",name.c_str(), name.length()))
    {
        frames[0].slots[IY] = std::move(value);
    } else if (matchString("graph",name.c_str(), name.length()))
    {
        frames[0].slots[IGRAPH] =  std::move(value);
    } else if (matchString("id",name.c_str(), name.length()))
    {
        frames[0].slots[IID] =  std::move(value);
    }
}

void default_instance_create_hook(Instance *instance)
{
   // INFO("Create instance: %s", instance->name.c_str());
}
void default_instance_destroy_hook(Instance *instance)
{
    //INFO("Destroy instance: %s", instance->name.c_str());
}
void default_instance_pre_execute_hook(Instance *instance)
{
   // INFO("Pre execute instance: %s", instance->name.c_str());
}
void default_instance_pos_execute_hook(Instance *instance)
{
   // INFO("Pos execute instance: %s", instance->name.c_str());
}
void default_process_exec_hook(Instance *instance)
{
   // INFO("Process exec instance: %s", instance->name.c_str());
}
