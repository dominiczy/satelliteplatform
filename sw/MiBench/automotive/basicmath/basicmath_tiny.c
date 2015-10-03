/**
 * @file   basicmath_tiny.c
 *
 * @author Dominic Zijlstra <dominiczijlstra@gmail.com> 
 * @date   Mon Jun 1 13:29:20 2015
 *
 * @brief  Added routine to print floats
 * 
 */

#include "snipmath.h"
#include <math.h>
#include <stdio.h>
/* The printf's may be removed to isolate just the math calculations */

/* This is a downsized version of basicmath_small.c */
void printnr(unsigned short int num)
    {
	if (num<8){
		if (num==0) write(1,"0",1);
		else if (num==1) write(1,"1",1);
		else if (num==2) write(1,"2",1);
		else if (num==3) write(1,"3",1);
		else if (num==4) write(1,"4",1);
		else if (num==5) write(1,"5",1);	
		else if (num==6) write(1,"6",1);
		else if (num==7) write(1,"7",1);	
	}else{ //only works with this bizarre construct, depending on O flag!
		if (num<9) write(1,"8",1);
		else write(1,"9",1);  
		
	}
      }

void fprint(double n){ //dominic zijlstra, function to print float

	unsigned short int digit;
	int m, m1;
        //char *c = s;
        m = log10(n);
	if (m < 1.0) {
            m = 0;
        }
	double PRECISION = 0.000001;
	while (n > PRECISION || m >= 0) {
            double weight = pow(10.0, m);
            if (weight > 0 && !isinf(weight)) {
                digit = floor(n / weight);
                n -= (digit * weight);
                printnr(digit);
            }
            if (m == 0 && n > 0)
                write(1,".",1);
            m--;
        }
}

void iprint(int n) //dominic zijlstra, function to print int
  { 
    if( n > 9 )
      { int a = n / 10;

        n -= 10 * a;
        iprint(a);
      }
	printnr(n);
  }

int main(void)
{
  double  a1 = 1.0, b1 = -10.5, c1 = 32.0, d1 = -30.0;
  double  a2 = 1.0, b2 = -4.5, c2 = 17.0, d2 = -30.0;
  double  a3 = 1.0, b3 = -3.5, c3 = 22.0, d3 = -31.0;
  double  a4 = 1.0, b4 = -13.7, c4 = 1.0, d4 = -35.0;
  double  x[3];
  double X;
  int     solutions;
  int i;
  unsigned long l = 0x3fed0169L;
  struct int_sqrt q;
  long n = 0;

  /* solve soem cubic functions */
  write(1,"********* CUBIC FUNCTIONS ***********\n",38);
  /* should get 3 solutions: 2, 6 & 2.5   */
  SolveCubic(a1, b1, c1, d1, &solutions, x);  
  write(1,"Solutions:",10);

  for(i=0;i<solutions;i++)  
  {
	fprint(x[i]);
	write(1,"\n",1);
  }
  /* should get 1 solution: 2.5           */
  SolveCubic(a2, b2, c2, d2, &solutions, x);  
  write(1,"Solutions:",10);
  for(i=0;i<solutions;i++)  
  {
	fprint(x[i]);
	write(1,"\n",1);
  }
  SolveCubic(a3, b3, c3, d3, &solutions, x);
  write(1,"Solutions:",10);
  for(i=0;i<solutions;i++)  
  {
	fprint(x[i]);
	write(1,"\n",1);
  }

  SolveCubic(a4, b4, c4, d4, &solutions, x);
  write(1,"Solutions:",10);
   for(i=0;i<solutions;i++)  
  {
	fprint(x[i]);
	write(1,"\n",1);
  }
  /* Now solve some random equations */
  for(a1=1;a1<5;a1++) {
    for(b1=4;b1>0;b1--) {
      for(c1=5;c1<8;c1+=0.5) {
	for(d1=-1;d1>-4;d1--) {
	  SolveCubic(a1, b1, c1, d1, &solutions, x);  
	  write(1,"Solutions:",10);
	  for(i=0;i<solutions;i++)  
  	  {
		fprint(x[i]);
		write(1,"\n",1);
          }
	}
      }
    }
  }
  
  write(1,"********* INTEGER SQR ROOTS ***********\n",100);
  /* perform some integer square roots */
  for (i = 0; i < 100; ++i)
    {
      usqrt(i, &q);
			// remainder differs on some machines
     // write(1,"sqrt(%3d) = %2d, remainder = %2d\n",
     write(1,"sqrt(",5);
	iprint(i);
	write(1,") = ",4);
	iprint(q.sqrt);
	 write(1,"\n",1);
    }
  usqrt(l, &q);
  //write(1,"\nsqrt(%lX) = %X, remainder = %X\n", l, q.sqrt, q.frac);
 
	write(1,"sqrt(",5);
	iprint(l);
	write(1,") = ",4);
	iprint(q.sqrt);
	 write(1,"\n",1);

  write(1,"********* ANGLE CONVERSION ***********\n",39);
  /* convert some rads to degrees */
  for (X = 0.0; X <= 360.0; X += 1.0)
 {
	fprint(X);
	write(1," degrees = ",11);
	fprint(deg2rad(X));
	write(1," radians\n",9);
}
  for (X = 0.0; X <= (2 * PI + 1e-6); X += (PI / 180))

   {
	fprint(X);
	write(1," radians = ",11);
	fprint(rad2deg(X));
	write(1," degrees\n",9);
}
  
  return 0;
}
