/*
 *  File : onDemandTrap.c
 *  Author : Francis Deslauriers fdeslaur@gmail.com
 */
#include <stdio.h>
int main()
{
	printf("X86_TRAP_BP, INT3\n");
	asm("INT3");
	return 0;
}
