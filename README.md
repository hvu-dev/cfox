# Virtual Machine

## Notes:
- Along with OP Code we can store value (e.g: integers), called immediate instructions, however it might only be suitable for constants of small and fixed-size variable.
- In a native compiler to machine code, bigger constants get stored in separate "constant data" region in the binary executable. VMs also do something similar to this (e.g: [JVM associates constant pool](https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html#jvms-4.4). 
- The idea of constant pool is that at compile time if there are multiple variables using the same values, for example:
```
// In C, we can call the action reusing the same memory address "string interning"
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
### Hash Table
- In this implementation (craftinginterpreters approach), we are going to use [FNV-1a](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash) function to hash input key.
- Open Address and Linear Probing was also chosen which is generally O(n) runtime to search a key in the worst case if there are many key fall into the same bucket and O(1) in the best case if a key is found at the calculated index.
- When delete an element in a hash table, we choose to treat the "deleted entry" as a full bucket and set it to a special value (which was called "tombstone" by the author). During iteration through all entries of the hash table, we will can either skip it or reuse it in the case of retreive and set, respectively. 
