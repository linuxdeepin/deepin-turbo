#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>


int main(void) {
  timeval tim;
  gettimeofday(&tim, NULL);
  printf("%d.%06d\n", 
	 static_cast<int>(tim.tv_sec), static_cast<int>(tim.tv_usec));
  return true;
}
