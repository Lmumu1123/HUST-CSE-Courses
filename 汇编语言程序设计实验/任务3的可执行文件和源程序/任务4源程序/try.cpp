#include<stdio.h>

int ditui1(int n);
int main(void) {
	int n = 0;
	printf("请输入n：");
	scanf_s("%d", &n);
	int result = ditui1(n);
	printf("第f(%d)项数列的值为：%d" ,n, result);
	return 0;
}
int ditui1(int n) {
	int result;
	__asm {
		mov ecx, n
		mov ebx, 0
		mov edx, 0
		call ditui2
		jmp next1
		ditui2:
		cmp ecx, 2
		je two
		cmp ecx, 1
		je one
		cmp ecx, 0
		je zero
		dec ecx
		call ditui2
		mov edx, eax
		add eax, ebx
		mov ebx, edx
		ret
		one :
		mov eax, 1
			ret
			two :
		mov eax, 2
			mov ebx, 1
			ret
			zero :
		mov eax, 0
			ret
			next1:
		mov result, eax
	}
	return result;
}
