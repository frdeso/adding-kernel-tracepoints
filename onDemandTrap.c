/*
 *  File : onDemandTrap.c
 *  Author : Francis Deslauriers fdeslaur@gmail.com
 */
#include <stdio.h>
#include <stdlib.h>
int main()
{
	printf("X86_TRAP_BP, INT3\n");
	asm("INT3");
	printf("X86_DIV_0\n");
	//char zero[1] = '0';
	int a = 1, b = atoi((const char*)'0');
	float c = (float)a/(float)b;
	printf("%f\n",c);


	return 0;
}
