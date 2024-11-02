#include<stdio.h>
#include<math.h>
#define K 32
char a[K];
int main()
{
	
	int x, n = 0;
	scanf("%d", &x);

	for (n = 0; pow(2, n) < x; n++);//将n初始化到2^n刚好大于等于x时
		if (pow(2, n) == x)//如果刚好等于
		{
			a[K - n - 1] = 1;
			for (int i = 0; i < K; i++)
			{
				printf("%d", a[i]);
			}
		}
		else //如果不等于
		{
			a[K-1] = x % 2;
			n -= 1;
			for (n; n >= 1; n--)
			{
				a[K - n-1] = (int)(x / pow(2, n));
				x = x % (int)pow(2, n);
			}
			for (int i = 0; i < K; i++)
			{
				printf("%d", a[i]);
			}
		}
    for(int i = K-1;i >= 0&& x ;i--){
        n = x % 2;
        x/=2;
        a[i] = n +'0';
    }
    printf("%s\n",a);
	return 0;
}