
#include "pch.h"

#include "Config.hpp"
#include "Vm.hpp"

extern void printValue(const Value &v);
extern void debugValue(const Value &v);
extern void printValueln(const Value &v);

void Process::set_parent(Process *p)
{
    // father = p;
    // if (p)
    // {
        
    //     if (p->son)
    //     {
    //         p->son->bigBrother = this;
            
    //         this->smallBrother = p->son;
    //     }
        
        
    //     p->son = this;
    // }
}

Process::Process(VirtualMachine *vm, const char *name) : Task(vm, name)
{
    type = TaskType::TPROCESS;
    prev = nullptr;
    next = nullptr;
    priority  = 0;
    isCreated = false;
  
    instance.ID = ID;
    instance.name = name;
    instance.user_data = nullptr;

    bigBrother=nullptr;
    smallBrother= nullptr;
    son = nullptr;
    father = nullptr;
    


   // INFO("Create process: %s", name);

}

Process::~Process()
{
 
   
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




//***************************************************************************************************************** */
//***************************************************************************************************************** */
//***************************************************************************************************************** */

ProcessList::ProcessList()
{
    head = nullptr;
    tail = nullptr;
    m_count = 0;
}

ProcessList::~ProcessList()
{
    Process *p = head;
    while (p)
    {
        Process *next = p->next;
        delete p;
        p = next;
    }

    head = nullptr;
    tail = nullptr;
    m_count = 0;
}

Process *ProcessList::by_priority(u32 priority)
{
    Process *p = head;
    while (p)
    {
        if (priority>=p->priority) return p;
        p = p->next;
    }
    return nullptr;
}

Process *ProcessList::by_name(const char *name)
{
    Process *p = head;
    while (p)
    {
        if(matchString(name, p->name.c_str(), p->name.length()))
        {
            return p;
        }
        p = p->next;
    }
    return nullptr;
}

void ProcessList::clear(bool freeData)
{
    Process *p = head;
    while (p)
    {
        Process *next = p->next;
        if (freeData)
        {
            delete p;
        }
        p = next;
    }
    head = nullptr;
    tail = nullptr;
    m_count = 0;
}

void ProcessList::add(Process *p)
{
    if (!p) return;

    if (!head)
    {
        head = p;
        tail = p;
    }
    else
    {
        p->prev = tail;
        tail->next = p;
        tail = p;
    }

    ++m_count;
}

void ProcessList::insert(Process *p)
{
    if (!p) return;
    if (!head)
    {
        add(p);
    }
    else
    {
        Process *n = by_priority(p->priority);

        if (!n)
        {
            add(p);
        }
        else
        {
            p->next = n;
            p->prev = n->prev;
            n->prev->next = p;
            n->prev = p;
        }
    }

    ++m_count;
}

bool ProcessList::remove(Process *n)
{
    if (!n) return false;
    if (!head) return false;
    if (n == head && n == tail) return false;
    if (n == head)
    {
        head = n->next;
        if (head != nullptr)
        {
            head->prev = nullptr;
        }
    }
    else
    {
        n->prev->next = n->next;
    }

    // Se o nó é o tail
    if (n == tail)
    {
        tail = n->prev;
        if (tail != nullptr)
        {
            tail->next = nullptr;
        }
    }
    else
    {
        if (n->next != nullptr)
        {
            n->next->prev = n->prev;
        }
    }


    --m_count; // Decrementa o contador de elementos

    return true; // Retorna o próximo elemento

}



//***************************************************************************************************************** */
//***************************************************************************************************************** */
//***************************************************************************************************************** */

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
