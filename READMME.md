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
- Each insutrction has a specific format: `{OPCODE}{OPERAND}` which takes 2 bytes
