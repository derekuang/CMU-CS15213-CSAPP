# 3 Machine-Level Representation of Program



## 3.60

- Problem

  > Consider the following assembly code:

  ```assembly
  # long loop(long x, int n)
  # x in %rdi, n in %esi
  loop:
  	movl	%esi, %ecx
  	movl	$1, %edx
  	movl	$0, %eax
  	jmp		.L2
  .L3:
  	movq	%rdi, %r8
  	andq	%rdx, %r8
  	orq		%r8, %rax
  	salq	%cl, %rdx
  .L2:
  	testq	%rdx, %rdx
  	jne		.L3
  	rep; ret
  ```

  > The preceding code was generated by compiling C code that had the following
  > overall form:

  ```c
  long loop(long x, long n)
  {
  	long result = ________;
  	long mask;
  	for (mask = ________; mask ________; mask = ________) {
  		result |= ________;
  	}
  	return result;
  }
  ```

  > Your task is to fill in the missing parts of the C code to get a program equivalent
  > to the generated assembly code. Recall that the result of the function is returned
  > in register %rax. You will find it helpful to examine the assembly code before,
  > during, and after the loop to form a consistent mapping between the registers and
  > the program variables.
  >
  > A. Which registers hold program values x, n, result, and mask?
  > B. What are the initial values of result and mask?
  > C. What is the test condition for mask?
  > D. How does mask get updated?
  > E. How does result get updated?
  > F. Fill in all the missing parts of the C code.

- Solution

  ```c
  long loop(long x, long n)
  {
  	long result = 0;
  	long mask;
  	for (mask = 1; mask != 0; mask = mask << (n & 0xff)) {
  		result |= mask & x;
  	}
  	return result;
  }
  ```



## 3.61

- Problem

  > In Section 3.6.6, we examined the following code as a candidate for the use of
  > conditional data transfer:

  ```c
  long cread(long *xp) {
  	return (xp ? *xp : 0);
  }
  ```

  > We showed a trial implementation using a conditional move instruction but argued
  > that it was not valid, since it could attempt to read from a null address.
  > 		Write a C function cread_alt that has the same behavior as cread, except
  > that it can be compiled to use conditional data transfer. When compiled, the
  > generated code should use a conditional move instruction rather than one of the
  > jump instructions.

- Solution

  ```c
  long cread_alt(long *xp) {
      return (!xp ? 0 : *xp);
  }
  ```

  

## 3.64

- Problem

  > Consider the following source code, where R, S, and T are constants declared with
  >
  > \#define:

  ```c
  long A[R][S][T];
  
  long store_ele(long i, long j, long k, long *dest)
  {
  	*dest = A[i][j][k];
  	return sizeof(A);
  }
  ```

  > In compiling this program, gcc generates the following assembly code:

  ```assembly
  ;long store_ele(long i, long j, long k, long *dest)
  ;i in %rdi, j in %rsi, k in %rdx, dest in %rcx
  store_ele:
  	leaq	(%rsi,%rsi,2), %rax
  	leaq	(%rsi,%rax,4), %rax
  	movq	%rdi, %rsi
  	salq	$6, %rsi
  	addq	%rsi, %rdi
  	addq	%rax, %rdi
  	addq	%rdi, %rdx
  	movq	A(,%rdx,8), %rax
  	movq	%rax, (%rcx)
  	movl	$3640, %eax
  	ret
  ```

  > A. Extend Equation 3.1 from two dimensions to three to provide a formula for
  > the location of array element A\[i]\[j][k].
  >
  > B. Use your reverse engineering skills to determine the values of R, S, and T
  > based on the assembly code.

- Solution

  > A. &A\[i]\[j][k] = x~A~ + L * (S * T * i + T * j + k)
  >
  > B. R = 7; S = 5; T = 13



## 3.68

- Problem

  > In the following code, A and B are constants defined with #define:

  ```c
  typedef struct {
  	int x[A][B]; /* Unknown constants A and B */
  	long y;
  } str1;
  
  typedef struct {
  	char array[B];
  	int t;
  	short s[A];
  	long u;
  } str2;
  
  void setVal(str1 *p, str2 *q) {
  	long v1 = q->t;
  	long v2 = q->u;
  	p->y = v1+v2;
  }
  ```

  > Gcc generates the following code for setVal:

  ```assembly
  ;void setVal(str1 *p, str2 *q)
  ;p in %rdi, q in %rsi
  setVal:
  	movslq	8(%rsi), %rax
  	addq 	32(%rsi), %rax
  	movq	%rax, 184(%rdi)
  	ret
  ```

  > What are the values of A and B? (The solution is unique.)

- Solution

  > A = 9; B = 5



## 3.69

- Problem

  > You are charged with maintaining a large C program, and you come across the
  > following code:

  ```c
  typedef struct {
  	int first;
  	a_struct a[CNT];
  	int last;
  } b_struct;
  
  void test(long i, b_struct *bp)
  {
  	int n = bp->first + bp->last;
  	a_struct *ap = &bp->a[i];
  	ap->x[ap->idx] = n;
  }
  ```

  > ​		The declarations of the compile-time constant CNT and the structure a_struct
  > are in a file for which you do not have the necessary access privilege. Fortunately,
  > you have a copy of the .o version of code, which you are able to disassemble with
  > the objdump program, yielding the following disassembly:

  ```assembly
  ;void test(long i, b_struct *bp)
  ;i in %rdi, bp in %rsi
  0000000000000000 <test>:
  	0:	8b 8e 20 01 00 00	mov		0x120(%rsi),%ecx
  	6:	03 0e				add		(%rsi),%ecx
  	8:	48 8d 04 bf			lea		(%rdi,%rdi,4),%rax
  	c:	48 8d 04 c6			lea		(%rsi,%rax,8),%rax
  	10:	48 8b 50 08			mov		0x8(%rax),%rdx
  	14:	48 63 c9			movslq 	%ecx,%rcx
  	17:	48 89 4c d0 10		mov		%rcx,0x10(%rax,%rdx,8)
  	1c:	c3					retq
  ```

  > ​	Using your reverse engineering skills, deduce the following:
  > A. The value of CNT.
  > B. A complete declaration of structure a_struct. Assume that the only fields
  > in this structure are idx and x, and that both of these contain signed values.

- Solution

  > A. CNT = 7

  ```c
  // B
  typedef struct {
      long idx;
      long x[4];
  } a_struct;
  ```

  
