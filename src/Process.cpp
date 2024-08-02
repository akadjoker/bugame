
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
        
        if (p->son)
        {
            p->son->bigBrother = this;
            
            this->smallBrother = p->son;
        }
        
        
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


    instance.locals[IX]     =   stack[IX].number;
    instance.locals[IY]     =   stack[IY].number;
    instance.locals[IGRAPH] =   stack[IGRAPH].number;
    //instance.locals[IGRAPH] = static_cast<s32>(AS_NUMBER(graph));
    vm->hooks.instance_pre_execute_hook(&instance);





   //printf("Process: %d\n", frames);
   if ( !isCreated)
   {
        create();
        return;
   }

    if (!isCreated) return;


    




    vm->hooks.process_exec_hook(&instance);


    stack[IX].number = instance.locals[IX];
    stack[IY].number = instance.locals[IY];
    stack[IGRAPH].number = instance.locals[IGRAPH];


    vm->hooks.instance_pos_execute_hook(&instance);
   
   
}

void Process::remove()
{
    Done();
    unmark();
    vm->hooks.instance_destroy_hook(&instance);
}

void Process::set_defaults()
{
    // addConstString("id");
    // addConstString("graph");
    // addConstString("x");
    // addConstString("y");
    //angle, father, file, flags, graph, xgraph, region, resolution, size, son, x, y, z, ctype, cnumber, priority

    push(INTEGER(ID));
    push(INTEGER(2));
    push(NUMBER(3));
    push(NUMBER(4));

    setLocalVariable("id", IID);
    setLocalVariable("graph", IGRAPH);
    setLocalVariable("x", IX);
    setLocalVariable("y", IY);


    instance.locals[IX]     =   stack[IX].number;
    instance.locals[IY]     =   stack[IY].number;
    instance.locals[IGRAPH] =   stack[IGRAPH].number;

}

void Process::set_variable(const String &name, Value value)
{
    // if (matchString("x",name.c_str(), name.length()))
    // {
    //     frames[0].slots[IX] = std::move(value);
    // } else if (matchString("y",name.c_str(), name.length()))
    // {
    //     frames[0].slots[IY] = std::move(value);
    // } else if (matchString("graph",name.c_str(), name.length()))
    // {
    //     frames[0].slots[IGRAPH] =  std::move(value);
    // } else if (matchString("id",name.c_str(), name.length()))
    // {
    //     frames[0].slots[IID] =  std::move(value);
    // }
}

void default_instance_create_hook(Instance *instance)
{
    INFO("Create instance: %s", instance->name.c_str());
}
void default_instance_destroy_hook(Instance *instance)
{
    INFO("Destroy instance: %s", instance->name.c_str());
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
