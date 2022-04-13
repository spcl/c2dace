#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
double a=0;
int n=10;
int i;
  for(i=0;i<n;i++){
	if(i==9) continue;
	a=i;

}

printf("First: %f\n",a);
  for(int i=0;i<n;i++){
	a=i;
	if (i==5) {break;}

}
if (a==10)
{a=5;
//return -1;
}
printf("Second: %f\n",a);

  return 0;

}
