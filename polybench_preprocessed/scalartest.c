#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

void foo(double *a,double c)
{
double b=4;
double c2;
b+=7;
c2=b+c;
*a=3.14;
c=c2+2;
}

int main(int argc, char **argv)
{
double a=0;
double b=1;
foo(&a,b);
printf("First: %f %f\n",a,b);

  return 0;

}
