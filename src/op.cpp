
#include "pch.h"
#include "Vm.hpp"
#include "Parser.hpp"

extern void printValue(const Value &v);
extern void debugValue(const Value &v);
extern void printValueln(const Value &v);

u8 Task::op_add()
{
            Value b = pop();
            Value a = pop();
            if (IS_NUMBER(a) && IS_NUMBER(b))
            {

                Value result = NUMBER(AS_NUMBER(a) + AS_NUMBER(b));
                push(std::move(result));
            }
            else if (IS_STRING(a) && IS_STRING(b))
            {
                String result = a.string->string + b.string->string;
                push(std::move(STRING(result.c_str())));
            }
            else if (IS_STRING(a) && IS_NUMBER(b))
            {
                String number(AS_NUMBER(b));
                String result = a.string->string + number;
                push(std::move(STRING(result.c_str())));
            }
            else if (IS_NUMBER(a) && IS_STRING(b))
            {
                String number(AS_NUMBER(a));
                String result = number + b.string->string;
                push(std::move(STRING(result.c_str())));
            }
            else
            {
                vm->Error("invalid 'adding' operands");
                printValue(a);
                printValue(b);
                
                return ABORTED;
            }

            return OK;
}

u8 Task::op_not_equal()
{
            Value b = pop();
            Value a = pop();
            if (IS_NUMBER(a) && IS_NUMBER(b))
            {
                Value result = BOOLEAN(AS_NUMBER(a) != AS_NUMBER(b));
                push(std::move(result));
            }
            else if (IS_STRING(a) && IS_STRING(b))
            {
                Value result = BOOLEAN(a.string->string != b.string->string);
                push(std::move(result));
            }
            else if (IS_BOOLEAN(a) && IS_BOOLEAN(b))
            {
                Value result = BOOLEAN(AS_BOOLEAN(a) != AS_BOOLEAN(b));
                push(std::move(result));
            }
            else
            {
                vm->Error("invalid 'not equal' operands.");
                printValue(a);
                printValue(b);
                return ABORTED;
            }

            return OK;
}

u8 Task::op_less()
{
            Value b = pop();
            Value a = pop();
            if (IS_NUMBER(a) && IS_NUMBER(b))
            {
                Value result = BOOLEAN(AS_NUMBER(a) < AS_NUMBER(b));
                push(std::move(result));
            }
            else if (IS_STRING(a) && IS_STRING(b))
            {
                Value result = BOOLEAN(a.string->string.length() < b.string->string.length());
                push(std::move(result));
            }
            else
            {
                vm->Error("invalid 'less' operands.");
                printValue(a);
                printValue(b);
                return ABORTED;
            }

            return OK;
}

u8 Task::op_greater()
{
            Value b = pop();
            Value a = pop();
            if (IS_NUMBER(a) && IS_NUMBER(b))
            {
                Value result = BOOLEAN(AS_NUMBER(a) > AS_NUMBER(b));
                push(std::move(result));
            }
            else if (IS_STRING(a) && IS_STRING(b))
            {
                Value result = BOOLEAN(a.string->string.length() > b.string->string.length());
                push(std::move(result));
            }
            else
            {
                vm->Error("invalid 'greater' .");
                printValue(a);
                printValue(b);

                return ABORTED;
            }


            return OK;
}

u8 Task::op_less_equal()
{
        Value b = pop();
            Value a = pop();
            if (IS_NUMBER(a) && IS_NUMBER(b))
            {
                Value result = BOOLEAN(AS_NUMBER(a) <= AS_NUMBER(b));
                push(std::move(result));
            }
            else if (IS_STRING(a) && IS_STRING(b))
            {
                Value result = BOOLEAN(a.string->string.length() <= b.string->string.length());
                push(std::move(result));
            }
            else
            {
                vm->Error("invalid 'less equal' .");
                printValue(a);
                printValue(b);

                return ABORTED;
            }

            return OK;
}

u8 Task::op_greater_equal()
{
            Value b = pop();
            Value a = pop();
            if (IS_NUMBER(a) && IS_NUMBER(b))
            {
                Value result = BOOLEAN(AS_NUMBER(a) >= AS_NUMBER(b));
                push(std::move(result));
            }
            else if (IS_STRING(a) && IS_STRING(b))
            {
                Value result = BOOLEAN(a.string->string.length() >= b.string->string.length());
                push(std::move(result));
            }
            else
            {
                vm->Error("invalid 'greater equal' .");
                printValue(a);
                printValue(b);

                return ABORTED;
            }

            return OK;
}

u8 Task::op_xor()
{
 Value b = pop();
            Value a = pop();
            if (IS_BOOLEAN(a) && IS_BOOLEAN(b))
            {
                bool b_a = AS_BOOLEAN(a);
                bool b_b = AS_BOOLEAN(b);
                bool result = b_a ^ b_b;
                push(std::move(BOOLEAN(result)));
            }
            else if (IS_NUMBER(a) && IS_NUMBER(b))
            {
                int n_a = static_cast<int>(AS_NUMBER(a));
                int n_b = static_cast<int>(AS_NUMBER(b));
                int result = n_a ^ n_b;
                double number = static_cast<double>(result);
                push(std::move(NUMBER(number)));
            }
            else if (IS_BOOLEAN(a) && IS_NUMBER(b))
            {
                bool b_a = AS_BOOLEAN(a);
                int n_b = static_cast<int>(AS_NUMBER(b));
                bool result = b_a ^ n_b;
                push(std::move(BOOLEAN(result)));
            }
            else if (IS_NUMBER(a) && IS_BOOLEAN(b))
            {
                int n_a = static_cast<int>(AS_NUMBER(a));
                bool b_b = AS_BOOLEAN(b);
                bool result = n_a ^ b_b;
                push(std::move(BOOLEAN(result)));
            }
            else
            {
                vm->Error("invalid 'xor' operands .");

                printValue(a);
                printValue(b);

                return ABORTED;
            }

            return OK;
}



u8 Task::op_mod(int line)
{
            Value b = pop();
            Value a = pop();
            if (IS_NUMBER(a) && IS_NUMBER(b))
            {
                double _a = AS_NUMBER(a);
                double divisor = AS_NUMBER(b);
                if (divisor == 0)
                {
                    vm->Error("division by zero [line %d]", line);

                    return ABORTED;
                }
                double result = fmod(_a, divisor);
                if (result != 0 && ((_a < 0) != (divisor < 0)))
                {
                    result += divisor;
                }
                push(std::move(NUMBER(result)));
            
            }
            else
            {
                vm->Error("invalid 'mod' operands [line %d]", line);

                return ABORTED;
            }
        return OK;
}
