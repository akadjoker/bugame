
#include "pch.h"
#include "Types.hpp"
#include "Vm.hpp"

static inline size_t string_hash(const char *str)
{
    size_t hash = 2166136261u;
    while (*str)
    {
        hash ^= (uint8_t)(*str++);
        hash *= 16777619;
    }
    return hash;
}

#define BUFFER_SIZE 64

static inline const char *doubleToString(double value)
{
    static char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "%f", value);
    return buffer;
}

static inline const char *longToString(long value)
{
    static char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "%ld", value);
    return buffer;
}

template <class T1, class T2>
inline void Swap(T1 &a, T2 &b)
{
    T1 c(a);
    a = b;
    b = c;
}

Traceable::Traceable()
{
    marked = false;
    id = 0;
    next = nullptr;
    prev = nullptr;
    type = ObjectType::UNDEFINED;
   //  printf("Create Traceable\n");
    Arena::as().queue(this);
}

Traceable::~Traceable()
{
   //   printf("Destroy Traceable %lld\n",id);
}

void *Traceable::operator new(size_t size)
{
    return Arena::as().allocate(size);
}

void Traceable::operator delete(void *ptr, size_t size)
{
    Arena::as().deallocate(ptr, size);
}

Arena::Arena()
{
    bytesAllocated = 0;
    totalObjects = 0;
    head = nullptr;
    tail = nullptr;
    next_id = 0;
    treshold = 1024 * 8;
    roots.reserve(1024 * 4);
    start_time = clock();
    headRemove = nullptr;
}

Arena::~Arena()
{
}

double Arena::get_elapsed_time()
{
    clock_t end_time = clock();
    return ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
}

void Arena::remove(Traceable *n)
{
    if (n == nullptr)
        return;

    // Se o nó é o head
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

    delete n;
}

void Arena::remove_root(Traceable *obj)
{
    roots.remove(obj);
    remove(obj);
}

void Arena::mark_from(Traceable *obj)
{

    if (obj == nullptr || obj->marked)
        return;

    obj->mark();
}

void Arena::mark()
{
    for (auto it = roots.begin(); it != roots.end(); ++it)
    {
        Traceable *obj = *it;
        mark_from(obj);
    }
}

void Arena::sweep()
{
    Traceable *temp = head;
    while (temp != nullptr)
    {
        if (!temp->marked)
        {
            Traceable *nodeToDelete = temp;
            temp = temp->next;
            remove_root(nodeToDelete);
        }
        else
        {
            temp->marked = false; // Resetar o marcador para o próximo ciclo
            temp = temp->next;
        }
    }
}

void Arena::gc()
{

    // int count = 0;

    // if (get_elapsed_time() >= 0.5)
    // {
    //    // printf(" %f \n", get_elapsed_time());
    //     start_time = clock();
    //     if (objects.size() > 0)
    //     {
    //         Traceable *temp = objects.pop();
    //         delete temp;
    //     }
    //     // Traceable *temp;
    //     // while (headRemove != nullptr)
    //     // {
    //     //     if (count > 100) break;
    //     //     temp = headRemove;
    //     //     headRemove = headRemove->next;
    //     //     delete temp;
    //     //     count++;
    //     //     // printf("GC: %lu objects, %lu bytes\n", (unsigned long)totalObjects, (unsigned long)bytesAllocated);
    //     // }

    // }

    if (bytesAllocated < treshold)
        return;

    mark();
    sweep();

    //  stats();

    //  stats();
}

void Arena::clear()
{
    roots.clear();
    Traceable *temp;
    while (head != nullptr)
    {
        temp = head;
        head = head->next;
        delete temp;
    }

    while (headRemove != nullptr)
    {

        temp = headRemove;
        headRemove = headRemove->next;
        delete temp;
    }

    stats();
}

void Arena::cleanup()
{
}

void Arena::stats()
{
    printf("---------Memory stats---------\n");

    printf("--Total objects:   %lu\n", (unsigned long)totalObjects);
    printf("--Bytes allocated: %lu\n", (unsigned long)bytesAllocated);
}

void *Arena::allocate(size_t size)
{

    bytesAllocated += size;
    totalObjects++;
    return ::operator new(size);
}

void Arena::deallocate(void *ptr, size_t size)
{

    bytesAllocated -= size;
    totalObjects--;

    //  printf("--Delete allocated: %lld\n", size);

    ::operator delete(ptr);
}

void Arena::queue(Traceable *obj)
{

    if (obj == nullptr)
        return;

    obj->id = next_id++;

    if (head == nullptr)
    {
        head = tail = obj;
    }
    else
    {
        obj->prev = tail;
        tail->next = obj;
        tail = obj;
    }
}

void List::reserve(size_t new_capacity)
{
    resize(new_capacity);
}

void List::resize(size_t size)
{
    Traceable **new_data = new Traceable *[size];
    std::memcpy(new_data, m_data, m_size * sizeof(Traceable *));
    delete[] m_data;
    m_data = new_data;
    m_capacity = size;
}

List::List(size_t capacity)
{

    m_capacity = capacity;
    m_size = 0;
    m_data = new Traceable *[m_capacity];
    std::memset(m_data, 0, m_capacity * sizeof(Traceable *));
}

List::~List()
{
    delete[] m_data;
    m_data = nullptr;
}

bool List::remove(Traceable *obj)
{

    if (obj == nullptr)
        return false;

    size_t left = 0;
    size_t right = m_size - 1;
    while (left <= right)
    {
        size_t mid = left + (right - left) / 2;
        if (m_data[mid]->id == obj->id)
        {
            // m_data[mid] = m_data[--m_size];
            remove_at(mid);
            return true;
        }
        else if (m_data[mid]->id < obj->id)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    return false;
}

void List::push_back(Traceable *obj)
{
    if (m_size == m_capacity)
    {
        reserve(m_capacity * 2);
    }
    m_data[m_size] = obj;
    m_size++;
}

Traceable *List::at(size_t index)
{
    if (index >= m_size)
    {
        return nullptr;
    }
    return m_data[index];
}

Traceable *List::remove_at(size_t index)
{
    Traceable *result = nullptr;
    if (index >= m_size)
    {
        return result;
    }

    result = m_data[index];
    for (size_t i = index; i < m_size - 1; i++)
    {
        m_data[i] = m_data[i + 1];
    }
    m_size--;

    return result;
}

bool List::contains(Traceable *obj)
{
    if (obj == nullptr)
        return false;

    int left = 0;
    int right = m_size - 1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        if (m_data[mid]->id == obj->id)
        {
            return true;
        }
        else if (m_data[mid]->id < obj->id)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    return false;
}

Traceable *List::pop()
{
    if (m_size == 0)
    {
        return nullptr;
    }
    return m_data[--m_size];
}

Set::Set(size_t initial_capacity)
{
    m_data.reserve(initial_capacity);
}

Set::~Set()
{
    m_data.clear();
}

bool Set::contains(Traceable *obj)
{
    if (obj == nullptr)
        return false;

    return m_data.contains(obj);
}

bool Set::add(Traceable *obj)
{
    if (obj == nullptr)
        return false;
    if (!contains(obj))
    {
        m_data.push_back(obj);
        return true;
    }
    return false;
}

bool Set::remove(Traceable *obj)
{
    if (obj == nullptr)
        return false;

    return m_data.remove(obj);
}

StringObject::StringObject(const String &str)
{
    string = str;
    type = ObjectType::OSTRING;
   // INFO("Create string: %s", string.c_str());
}

StringObject::StringObject(const char *str)
{
    string = String(str);
    type = ObjectType::OSTRING;
  //  INFO("Create string: %s", string.c_str());
}

StringObject::StringObject(double value)
{
    string = String(value);
    type = ObjectType::OSTRING;
}

StringObject::StringObject(int value)
{
    string = String(value);
    type = ObjectType::OSTRING;
}

StringObject::~StringObject()
{
    INFO("Delete string: %s", string.c_str());
}

NativeFunctionObject::NativeFunctionObject(NativeFunction func, const char *name, int arity) : func(func), name(name), arity(arity)
{
}

int NativeFunctionObject::call(VirtualMachine *vm, int argc, Value *args)
{
    return func(vm, argc, args);
}

Chunk::Chunk(u32 capacity)
    :  m_capacity(capacity), count(0)
{
    code  = (u8*)  std::malloc(capacity * sizeof(u8));
    lines = (int*) std::malloc(capacity * sizeof(int));

    //printf("create chunk   \n");
}

Chunk::Chunk(Chunk *other)
{

    code  = (u8*)  std::malloc(other->m_capacity * sizeof(u8));
    lines = (int*) std::malloc(other->m_capacity * sizeof(int));
    
    m_capacity = other->m_capacity;
    count = other->count;

    std::memcpy(code, other->code, other->m_capacity * sizeof(u8));
    std::memcpy(lines, other->lines, other->m_capacity * sizeof(int));
}


bool Chunk::clone(Chunk *other)
{
     if (!other)
        return false;

    
    std::free(other->code);
    std::free(other->lines);

    
    other->m_capacity = m_capacity;
    other->count = count;

    other->code = (u8*) std::malloc(m_capacity * sizeof(u8));
    other->lines = (int*) std::malloc(m_capacity * sizeof(int));

    if (!other->code || !other->lines)
    {
        std::free(other->code);
        std::free(other->lines);
        DEBUG_BREAK_IF(other->code == nullptr || other->lines == nullptr);
        return false;
    }

    std::memcpy(other->code, code, count * sizeof(u8));
    std::memcpy(other->lines, lines, count * sizeof(int));

    return true;
    
}


Chunk::~Chunk()
{
    std::free(code);
    std::free(lines);

  //  printf("destroy chunk  \n");
}

void Chunk::reserve(u32 capacity)
{
    if (capacity > m_capacity)
    {
       

        u8 *newCode  = (u8*) (std::realloc(code,  capacity * sizeof(u8)));
        int *newLine = (int*)(std::realloc(lines, capacity * sizeof(int)));

        if (!newCode || !newLine)
        {
            std::free(newCode);
            std::free(newLine);
            DEBUG_BREAK_IF(newCode == nullptr || newLine == nullptr);
            return;
        }



        code = newCode;
        lines = newLine;      
        m_capacity = capacity;
    }
}



void Chunk::write(u8 instruction, int line)
{
    if (m_capacity < count + 1)
    {
        int oldCapacity = m_capacity;
        m_capacity = GROW_CAPACITY(oldCapacity);
        u8 *newCode  = (u8*) (std::realloc(code,  m_capacity * sizeof(u8)));
        int *newLine = (int*)(std::realloc(lines, m_capacity * sizeof(int)));
        if (!newCode || !newLine)
        {
            std::free(newCode);
            std::free(newLine);
            DEBUG_BREAK_IF(newCode == nullptr || newLine == nullptr);
            return;
        }
        code = newCode;
        lines = newLine;      

    }
    
    code[count]  = instruction;
    lines[count] = line;
    count++;
}

u8 Chunk::operator[](u32 index)
{
    DEBUG_BREAK_IF(index > m_capacity);
    return code[index];
}
