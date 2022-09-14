# 10 System-Level I/O



## 10.6

- Problem

  > What is the output of the following program?

  ```c
  #include "csapp.h"
  
  int main()
  {
      int fd1, fd2;
      
      fd1 = Open("foo.txt", O_RDONLY, 0);
      fd2 = Open("bar.txt", O_RDONLY, 0);
      Close(fd2);
      fd2 = Open("baz.txt", O_RDONLY, 0);
      printf("fd2 = %d\n", fd2);
      exit(0);
  }
  ```

- Solution

  The answer is 4.



## 10.9

- Problem

  > Consider the following invocation of the fstatcheck program from Problem 10.8:
  >
  > linux> fstatcheck 3 < foo.txt
  >
  > You might expect that this invocation of fstatcheck would fetch and display metadata for file foo.txt. However, when we run it on our system, it fails with a “bad file descriptor.” Given this behavior, fill in the pseudocode that the shell must be executing between the fork and execve calls:

  ```c
  if (Fork() == 0) { /* child */
  	/* What code is the shell executing right here? */
  	Execve("fstatcheck", argv, envp);
  }
  ```

- Solution

  ```c
  if (Fork() == 0) { /* child */
  	dup2(STDIN_FILENO, atoi(argv[1])); // dup2(0, 3);
  	Execve("fstatcheck", argv, envp);
  }
  ```

  

## 10.10

- Problem

  > Modify the cpfile program in Figure 10.5 so that it takes an optional command-line argument infile. If infile is given, then copy infile to standard output; otherwise, copy standard input to standard output as before. The twist is that your solution must use the original copy loop (lines 9–11) for both cases. You are only allowed to insert code, and you are not allowed to change any of the existing code.

- Solution

  ```c
  #include "csapp.h"
  
  int main(int argc, char **argv)
  {
      int n;
      rio_t rio;
      char buf[MAXLINE];
      
      char *infile;
      int fd;
      if (argc > 1) {
          infile = argv[1];
          if ((fd = Open(infile, O_RDONLY, 0777)) > 0) {
              Dup2(fd, STDIN_FILENO);
          }
      }
      
      Rio_readinitb(&rio, STDIN_FILENO);
      while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
          Rio_writen(STDOUT_FILENO, buf, n);
  }
  ```

  