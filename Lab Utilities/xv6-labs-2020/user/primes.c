#include "kernel/types.h"
#include "user/user.h"

//The first process feeds the numbers 2 through 35 into the pipeline
//It's simplest to directly write 32-bit (4-byte) ints to the pipes, rather than using formatted ASCII I/O.
void source() {
  int i;
  for (i = 2; i < 36; i++) {
    write(1, &i, sizeof(i));
  }
}

//Filter out numbers that cannot be divided by p and write the remaining numbers to the standard output.
void remove(int p) {
  int n;
  while (read(0, &n, sizeof(n))) {
    if (n % p != 0) {
      write(1, &n, sizeof(n));
    }
  }
}

//Redirects standard input or output to one end of the pipeline
void redirect(int k, int pd[]) {
  close(k); //Turn off standard input (0) or standard output (1)
  dup(pd[k]);//Copy one end of the pipeline to standard input or standard output
  close(pd[0]);
  close(pd[1]);
}

//Recursively creating new processes and pipelines to filter out prime numbers and print them
void recursioncom() {
  int pd[2];
  int p;

  if (read(0, &p, sizeof(p))) {
    printf("prime %d\n", p);
    pipe(pd);
    if (fork()) {
      //Parent  
      redirect(0, pd);
      recursioncom();
    } 
    else {
      //Child
      redirect(1, pd);
      remove(p);
    }
  }
}

int main(int argc, char *argv[]) {

  int pd[2];
  pipe(pd);

  if (fork()) {
    //Parent
    redirect(0, pd); //Repositioning standard input to pipeline in parent process
    recursioncom();
  } 
  else {
    //Child
    redirect(1, pd); //Repositioning standard output to pipeline in subprocess
    source();
  }

  exit(0);
}