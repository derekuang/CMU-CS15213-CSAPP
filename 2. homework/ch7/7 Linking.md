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

  > (a) REF(main.1) -> DEF(main.1)
  >
  > (b) REF(main.2) -> DEF(main.2)

  **B.**

  > (a) REF(x.1) -> UNKNOWN
  >
  > (b) REF(x.2) -> UNKNOWN

  **C.**

  > (a) REF(x.1) -> ERROR
  >
  > (b) REF(x.2) -> ERROE



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

  By rule2, main in foo6 is a strong symbol, and main in bar6 is a weak symbol, so the main in bar6.c reference the symbol in foo6, therefore p2 print the address of the main function.



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

  A. ADDR(S) = ADDR(.text) = 0x4004e0
  
  ​	 refaddr = ADDR(S) + r.offset = 0x4004ea
  
  ​	 *refptr = ADDR(r.symbol) - refaddr + r.addend = 0x4004f8 - 0x4004ea - 4 = 0xa
  
  B. ADDR(S) = ADDR(.text) = 0x4004d0
  
  ​	 refaddr = ADDR(S) + r.offset = 0x4004da
  
  ​	 *refptr = ADDR(r.symbol) - refaddr + r.addend = 0x400500 - 0x4004da - 4 = 0x22



## 7.13

- Problem

  > Performing the following tasks will help you become more familiar with the various tools for manipulating object files.
  >
  > A. How many object files are contained in the versions of libc.a and libm.a on your system?
  > B. Does gcc -Og produce different executable code than gcc -Og -g?
  > C. What shared libraries does the gcc driver on your system use?

- Solution

  A.

  > $ ar -t /usr/lib32/libc.a | wc -l
  >
  > 1982
  >
  > $ ar -t /usr/lib32/libm.a | wc -l
  >
  > 748

  B. Same.

  C.

  > $ ldd /bin/ls | sed 's/^ */    /'
  >
  > ​	linux-vdso.so.1 (0x00007fff16dad000)
  > ​	libselinux.so.1 => /lib/x86_64-linux-gnu/libselinux.so.1 (0x00007f9f8ccbb000)
  > ​	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f9f8ca93000)
  > ​	libpcre2-8.so.0 => /lib/x86_64-linux-gnu/libpcre2-8.so.0 (0x00007f9f8c9fc000)
  > ​	/lib64/ld-linux-x86-64.so.2 (0x00007f9f8cd1c000)