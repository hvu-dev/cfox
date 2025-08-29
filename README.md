# Virtual Machine

## Notes:
- Along with OP Code we can store value (e.g: integers), called immediate instructions, however it might only be suitable for constants of small and fixed-size variable.
- In a native compiler to machine code, bigger constants get stored in separate "constant data" region in the binary executable. VMs also do something similar to this (e.g: [JVM associates constant pool](https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html#jvms-4.4). 
- The idea of constant pool is that at compile time if there are multiple variables using the same values, for example:
```
const String a = "hello";
const String b = "hello";
```
These "hello" values are not likely to be changed at runtime, so they can use a single memory unit.
For example in Python, you can clearly see the interpreter "reuse" some values (point the variable to the same "id"), which works really well since they are immutable, modify a variable's value will create a new "data".
```
a = "hello"
b = "hello"
print(a is b) # print "True"
c = 1
d = 1
print(c is d) # print "True"
```
- Each insutrction has a specific format: `{OPCODE}{OPERAND}` which takes 2 bytes
- We can arrange instructions and use a program counter to guide the machine to the next instruction.
- The compiler can freely parse the OP code and change the structure to optimise performance or reduce source code. 
```
// This can be translated to: OPERAND OP_NOT_EQUAL OPERAND
a != b
// Or can be translated to: OP_NOT(OPERAND OP_EQUAL_EQUAL OPERAND)
// Which result in less required OP code in the compiler, in this case, OP_NOT_EQUAL is redundant.
```
- It's very important to free any object that is dynamically allocated, for example when we create strings, and concat tthose strings.
### Pratt Parsing
### Tagged Union
