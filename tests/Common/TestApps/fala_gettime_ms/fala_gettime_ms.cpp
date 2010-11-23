#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char **argv) {
  timeval tim;
  gettimeofday(&tim, NULL);
  if (argc > 0) {
      printf("%d%03d %s\n", 
             static_cast<int>(tim.tv_sec), static_cast<int>(tim.tv_usec/1000), argv[1]);
  } else {
      printf("%d%03d\n", 
          static_cast<int>(tim.tv_sec), static_cast<int>(tim.tv_usec/1000));
  }
  return true;
}
