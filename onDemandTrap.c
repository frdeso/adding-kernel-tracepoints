
#include <stdio.h>
int main()
{
	float a = 18, b = 0;
	float c = a/b;
	printf("a: %f , b = %f, result : %f\n", a,b,c);
	printf("X86_TRAP_BP, INT3\n");
	asm("INT3");
	return 0;
}
