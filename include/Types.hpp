#pragma once

#include "Config.hpp"
#include "Utils.hpp"
#include "String.hpp"
#include "Vector.hpp"
#include "Map.hpp"

class VirtualMachine;
struct Value;
class Task;

typedef int (*NativeFunction)(VirtualMachine *vm, int argc, Value *args);

struct Traceable
{
    bool marked;
    u64 id;
    Traceable *next;
    Traceable *prev;
    ObjectType type;

    Traceable();

    virtual ~Traceable();

    virtual void mark() { marked = true; }
    virtual void unmark() { marked = false; }

    static void *operator new(size_t size);
    static void operator delete(void *ptr, size_t size);
};

struct StringObject : public Traceable
{
    String string;
    StringObject(const String &str);
    StringObject(const char *str);
    StringObject(double value);
    StringObject(int value);
    ~StringObject();
};

struct NativeFunctionObject : public Traceable
{
    NativeFunction func;
    String name;
    int arity;
    NativeFunctionObject(NativeFunction func, const char *name, int arity);
    int call(VirtualMachine *vm, int argc, Value *args);
};

struct Value
{

    ValueType type;
    union
    {
        double number;
        StringObject *string;
        bool boolean;
        NativeFunctionObject *native;
    };
};

#define INTEGER(value) \
    (Value{ ValueType::VNUMBER, {.number = static_cast<double>(value)}})
#define NUMBER(value) \
    (Value{ ValueType::VNUMBER, {.number = value}})
#define STRING(value) \
    (Value{ ValueType::VSTRING, {.string = new StringObject(value)}})
#define BOOLEAN(value) \
    (Value{ ValueType::VBOOLEAN, {.boolean = value}})
#define NONE() \
    (Value{ ValueType::VNONE, {.number = 0}})
#define NATIVE(fn, name, arity) \
    (Value{ ValueType::VNATIVE, {.native = new NativeFunctionObject(fn, name, arity)}})

#define AS_INTEGER(value) (static_cast<int>((value).number))
#define AS_NUMBER(value) ((double)(value).number)
#define AS_BOOLEAN(value) ((bool)(value).boolean)
#define AS_STRING(value) ((StringObject *)(value).string)
#define AS_RAW_STRING(value) (AS_STRING(value)->string.c_str())
#define AS_NATIVE(value) ((NativeFunctionObject *)(value).native)

#define IS_BOOLEAN(value) ((value).type == ValueType::VBOOLEAN)
#define IS_NUMBER(value) ((value).type == ValueType::VNUMBER)
#define IS_STRING(value) ((value).type == ValueType::VSTRING)
#define IS_NONE(value) ((value).type == ValueType::VNONE)
#define IS_NATIVE_DEF(value) ((value).type == ValueType::VNATIVE)


#define IS_OBJECT(value) ((value).type == ValueType::VSTRING)
#define MARK(value)                  \
    {                                \
        if (IS_OBJECT(value))        \
            AS_STRING(value)->mark() \
    }
#define UNMARK(value)                  \
    {                                  \
        if (IS_OBJECT(value))          \
            AS_STRING(value)->unmark() \
    }

inline Value Clone(const Value &value)
{
    switch (value.type)
    {
    case ValueType::VNUMBER:
        return NUMBER(AS_NUMBER(value));
    case ValueType::VSTRING:
        return STRING(AS_STRING(value)->string);
    case ValueType::VBOOLEAN:
        return BOOLEAN(AS_BOOLEAN(value));
    default:
        return NONE();
    }
}

inline bool MatchValue(const Value &value, const Value &with)
{
    if (value.type != with.type)
        return false;
    if (IS_STRING(value) && IS_STRING(with))
    {
        if (AS_STRING(value)->string.length() != AS_STRING(with)->string.length())
            return false;
        return AS_STRING(value)->string == AS_STRING(with)->string;
    }
    else if (IS_NUMBER(value) && IS_NUMBER(with))
        return fabs(AS_NUMBER(value) - AS_NUMBER(with)) < 0.01953; // TODO: use epsilon error margin
    // return AS_NUMBER(value) == AS_NUMBER(with);
    else if (IS_BOOLEAN(value) && IS_BOOLEAN(with))
        return AS_BOOLEAN(value) == AS_BOOLEAN(with);

    else if (IS_NONE(value))
        return true;
    return false;
}

inline bool isFalsey(Value value)
{
    if (IS_NONE(value))
        return true;
    if (IS_BOOLEAN(value))
    {
        return !value.boolean;
    }
    if (IS_STRING(value))
        return AS_STRING(value)->string.length() == 0;
    if (IS_NUMBER(value))
    {
        int i = (int)AS_NUMBER(value);
        return (i == 0);
    }
    return false;
}

class Chunk
{
    u32 m_capacity;

public:
    Chunk(u32 capacity = 512);
    Chunk(Chunk *other);
    ~Chunk();

    void clear();
    void reserve(u32 capacity);

    void write(u8 instruction, int line);

    u32 capacity() const { return m_capacity; }

    u8 operator[](u32 index);

    bool clone(Chunk *other);

    u8 *code;
    int *lines;
    u32 count;
};
class List
{
private:
    Traceable **m_data;
    size_t m_capacity;
    size_t m_size;

    void resize(size_t size);

public:
    List(size_t capacity = 16);
    ~List();

    void reserve(size_t capacity);

    bool remove(Traceable *obj);
    Traceable *remove_at(size_t index);

    void push_back(Traceable *obj);
    Traceable *at(size_t index);

    bool contains(Traceable *obj);

    size_t size() { return m_size; }

    void clear() { m_size = 0; }
    bool empty() { return m_size == 0; }

    Traceable *pop();

    Traceable *operator[](size_t index) { return m_data[index]; }
    Traceable &operator[](size_t index) const { return *m_data[index]; }

    class Iterator
    {
    private:
        Traceable **m_ptr;

    public:
        Iterator(Traceable **ptr) : m_ptr(ptr) {}

        Iterator &operator++()
        {
            ++m_ptr;
            return *this;
        }

        Traceable *operator*()
        {
            return *m_ptr;
        }

        bool operator!=(const Iterator &other) const
        {
            return m_ptr != other.m_ptr;
        }
    };

    Iterator begin()
    {
        return Iterator(m_data);
    }

    Iterator end()
    {
        return Iterator(m_data + m_size);
    }
};

struct Entry
{
    uint32_t hash;
    String key;
    Value value;
    bool use;
    Entry()
    {
        value = NONE();
        hash = 0;
        use = false;
        key = "";
    };
};

class Set
{
private:
    List m_data;

public:
    Set(size_t initial_capacity = 10);
    ~Set();

    bool contains(Traceable *obj);

    bool add(Traceable *obj);

    bool remove(Traceable *obj);

    size_t size() { return m_data.size(); }
};

class Arena
{

private:
    size_t bytesAllocated;
    size_t totalObjects;
    size_t treshold;

    Traceable *head;
    Traceable *tail;

    Traceable *headRemove;

    clock_t start_time;

    u64 next_id;
    List objects;

    Arena();
    ~Arena();

    void remove_root(Traceable *obj); // remove from roots and delete
    void mark_from(Traceable *obj);
    void mark();
    void sweep();
    double get_elapsed_time();

public:
    static Arena &as()
    {
        static Arena arena;
        return arena;
    }

    void cleanup();
    void clear();

    void stats();

    void *allocate(size_t size);
    void deallocate(void *ptr, size_t size);
    void queue(Traceable *obj);
    void remove(Traceable *obj); // remove & delete

    void add(Traceable *obj) { roots.push_back(obj); }

    size_t get_total_objects() { return totalObjects; }

    size_t get_bytes_allocated() { return bytesAllocated; }

    void gc();

    List roots;
};
