// Demonstration program for testing CPP_parser.g

/* 1999-2004 Version 3.0 July 2004
 * Modified by David Wigg at London South Bank University for CPP_parser.g
 * 
 * See MyReadMe.txt for further information
 *
 * This file is best viewed in courier font with tabs set to 4 spaces
 *
 * Use Quadratic.i to verify installation of CPP_parser
 *
 * This calculates solutions for Ax^2+Bx+C = 0 for simple values of x
 *  e.g 1 6 -16
*/

#include <iostream>
#include <math.h>

double HR1,HR2,HI;

void RROOTS(double CSID, double DISC, double H1);

void ROOTS(double CSID, double DISC, double H1);

void main()
	{
	double A,B,C;
	double H1,HR,DISC,CSID;

	printf("To solve Ax^2 + Bx + C = 0\n");
	printf("enter three values for, A B and C:\n");
	scanf("%lf %lf %lf",&A,&B,&C);

	H1 = -B/(2.0*A);
	HR = H1*H1;
	DISC = HR-C/A;
	CSID = H1*H1-C/A;

	RROOTS(CSID,DISC,H1);

	printf("x = %f or %f\n",HR1,HR2);

	return;
	}

void RROOTS(double CSID,double DISC,double H1)
	{
	double H2;

	if (DISC>=0)
		{
		H2 = sqrt(DISC);
		HR1 = H1+H2;
		HR2 = H1-H2;
		HI = 0.0;
		}

	ROOTS(CSID,DISC,H1);

	return;
	}

void ROOTS(double CSID,double DISC,double H1)
	{
	double H2;

	if (CSID<0)
		{
		H2 = sqrt(-DISC);
		HR1 = H1;
		HR2 = H1;
		HI = H2;
		}

	return;
	}
