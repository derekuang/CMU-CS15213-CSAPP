# 7 Linking



## 7.8

- Problem

  > In this problem, let REF(x.i) → DEF(x.k) denote that the linker will associate an arbitrary reference to symbol x in module i to the definition of x in module k. For each example below, use this notation to indicate how the linker would resolve references to the multiply-defined symbol in each module. If there is a link-time error (rule 1), write “ERROR”. If the linker arbitrarily chooses one of the definitions (rule 3), write “UNKNOWN”.

  **A.**

  ```c
  /* Module1 */
  int main()
  {
  }
  
  /* Module2 */
  static int main=1;
  int p2()
  {
  }
  ```

  > (a) REF(main.1) -> DEF()
  >
  > (b) REF(main.2) -> DEF()

  **B.**

  ```c
  /* Module1 */
  int x;
  void main()
  {
  }
  
  /* Module2 */
  double x;
  int p2()
  {
  }
  ```

  > (a) REF(x.1) -> DEF()
  >
  > (b) REF(x.2) -> DEF()

  **C.**

  ```c
  /* Module1 */
  int x = 1;
  void main()
  {
  }
  
  /* Module2 */
  double x = 1.0;
  int p2()
  {
  }
  ```

  > (a) REF(x.1) -> DEF()
  >
  > (b) REF(x.2) -> DEF()

- Solution

  **A.**

  > (a) REF(main.1) -> DEF()
  >
  > (b) REF(main.2) -> DEF()

  **B.**

  > (a) REF(x.1) -> DEF()
  >
  > (b) REF(x.2) -> DEF()

  **C.**

  > (a) REF(x.1) -> DEF()
  >
  > (b) REF(x.2) -> DEF()



## 7.9

- Problem

  > Consider the following program, which consists of two object modules:

  ```c
  /* foo6.c */
  void p2(void);
  
  int main()
  {
      p2();
      return 0;
  }
  
  /* bar6.c */
  #include <stdio.h>
  
  char main;
  
  void p2()
  {
      printf("0x%x\n", main);
  }
  ```

  > ​	When this program is compiled and executed on an x86-64 Linux system, it prints the string 0x48\n and terminates normally, even though function p2 never initializes variable main. Can you explain this?

- Solution

  



## 7.12

- Problem

  > Consider the call to function swap in object file m.o (Problem 7.6).

  ```assembly
  9:	e8 00 00 00 00		callq	e <main+0xe>	swap()
  ```

  > with the following relocation entry:
  >
  > r.offset = 0xa
  > r.symbol = swap
  > r.type = R_X86_64_PC32
  > r.addend = -4
  >
  > A. Suppose that the linker relocates .text in m.o to address 0x4004e0 and swap to address 0x4004f8. Then what is the value of the relocated reference to swap in the callq instruction?
  > B. Suppose that the linker relocates .text in m.o to address 0x4004d0 and swap to address 0x400500. Then what is the value of the relocated reference to swap in the callq instruction?

- Solution

  

## 7.13

- Problem

  > Performing the following tasks will help you become more familiar with the various tools for manipulating object files.
  >
  > A. How many object files are contained in the versions of libc.a and libm.a on your system?
  > B. Does gcc -Og produce different executable code than gcc -Og -g?
  > C. What shared libraries does the gcc driver on your system use?

- Solution