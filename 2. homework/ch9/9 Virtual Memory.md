# 9 Virtual Memory



> Example memory system in Section 9.6.4:
>
> - The memory is byte addressable.
> - Memory accesses are to 1-byte words (not 4-byte words).
> - Virtual addresses are 14 bits wide (n = 14).
> - Physical addresses are 12 bits wide (m = 12).
> - The page size is 64 bytes (P = 64).
> - The TLB is 4-way set associative with 16 total entries.
> - The L1 d-cache is physically addressed and direct mapped, with a 4-byte line size and 16 total sets.

![](Figure 9.20.png)



## 9.11

- Problem

  > In the following series of problems, you are to show how the example memory system in Section 9.6.4 translates a virtual address into a physical address and accesses the cache. For the given virtual address, indicate the TLB entry accessed, the physical address, and the cache byte value returned. Indicate whether the TLB misses, whether a page fault occurs, and whether a cache miss occurs. If there is a cache miss, enter “—” for “Cache byte returned.” If there is a page fault, enter “—” for “PPN” and leave parts C and D blank.
  >
  > Virtual address: 0x027c

- Solution

  > A. Virtual address format

  |  13  |  12  |  11  |  10  |  9   |  8   |  7   |  6   |  5   |  4   |  3   |  2   |  1   |  0   |
  | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
  |  0   |  0   |  0   |  0   |  1   |  0   |  0   |  1   |  1   |  1   |  1   |  1   |  0   |  0   |

  > B. Address translation

  | Parameter         | Value |
  | ----------------- | ----- |
  | VPN               | 0x09  |
  | TLB index         | 0x1   |
  | TLB tag           | 0x02  |
  | TLB hit? (Y/N)    | N     |
  | Page fault? (Y/N) | N     |
  | PPN               | 0x17  |

  > C. Physical address format

  |  11  |  10  |  9   |  8   |  7   |  6   |  5   |  4   |  3   |  2   |  1   |  0   |
  | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
  |  0   |  1   |  0   |  1   |  1   |  1   |  1   |  1   |  1   |  1   |  0   |  0   |

  > D. Physical memory reference

  | Parameter           | Value |
  | ------------------- | ----- |
  | Byte offset         | 0x0   |
  | Cache index         | 0xf   |
  | Cache tag           | 0x17  |
  | Cache hit? (Y/N)    | N     |
  | Cache byte returned | —     |

  

## 9.12

- Problem

  > Repeat Problem 9.11 for the following address.
  >
  > Virtual address: 0x03a9

- Solution

  > A. Virtual address format

  |  13  |  12  |  11  |  10  |  9   |  8   |  7   |  6   |  5   |  4   |  3   |  2   |  1   |  0   |
  | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
  |  0   |  0   |  0   |  0   |  1   |  1   |  1   |  0   |  1   |  0   |  1   |  0   |  0   |  1   |

  > B. Address translation

  | Parameter         | Value |
  | ----------------- | ----- |
  | VPN               | 0x0e  |
  | TLB index         | 0x2   |
  | TLB tag           | 0x03  |
  | TLB hit? (Y/N)    | N     |
  | Page fault? (Y/N) | N     |
  | PPN               | 0x11  |

  > C. Physical address format

  |  11  |  10  |  9   |  8   |  7   |  6   |  5   |  4   |  3   |  2   |  1   |  0   |
  | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
  |  0   |  1   |  0   |  0   |  0   |  1   |  1   |  0   |  1   |  0   |  0   |  1   |

  > D. Physical memory reference

  | Parameter           | Value |
  | ------------------- | ----- |
  | Byte offset         | 0x1   |
  | Cache index         | 0xa   |
  | Cache tag           | 0x11  |
  | Cache hit? (Y/N)    | N     |
  | Cache byte returned | —     |

  

## 9.13

- Problem

  > Repeat Problem 9.11 for the following address.
  >
  > Virtual address: 0x0040

- Solution

  > A. Virtual address format

  |  13  |  12  |  11  |  10  |  9   |  8   |  7   |  6   |  5   |  4   |  3   |  2   |  1   |  0   |
  | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
  |  0   |  0   |  0   |  0   |  0   |  0   |  0   |  1   |  0   |  0   |  0   |  0   |  0   |  0   |

  > B. Address translation

  | Parameter         | Value |
  | ----------------- | ----- |
  | VPN               | 0x01  |
  | TLB index         | 0x1   |
  | TLB tag           | 0x00  |
  | TLB hit? (Y/N)    | N     |
  | Page fault? (Y/N) | Y     |
  | PPN               | —     |

  > C. Physical address format

  |  11  |  10  |  9   |  8   |  7   |  6   |  5   |  4   |  3   |  2   |  1   |  0   |
  | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
  |      |      |      |      |      |      |      |      |      |      |      |      |

  > D. Physical memory reference

  | Parameter           | Value |
  | ------------------- | ----- |
  | Byte offset         |       |
  | Cache index         |       |
  | Cache tag           |       |
  | Cache hit? (Y/N)    |       |
  | Cache byte returned |       |

  

## 9.14

- Problem

  > Given an input file hello.txt that consists of the string Hello, world!\n, write a C program that uses mmap to change the contents of hello.txt to Jello, world!\n.

- Solution

  ```c
  #include <stdlib.h>
  #include <sys/mman.h>
  #include <fcntl.h>
  
  int main()
  {
      int fd;
      char *bufp; /* ptr to memory-mapped VM area */
  
      fd = open("hello.txt", O_RDWR, 0);
      bufp = mmap(NULL, 1, PROT_WRITE, MAP_SHARED, fd, 0);
      bufp[0] = 'J';
  
      return 0;
  }
  ```

  