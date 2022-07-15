#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
double a=0;
int n=10;
  for(int i=0;i<n;i++){
	if(i==9) continue;
	a=i;

}
printf("First: %f\n",a);
  return 0;

}
