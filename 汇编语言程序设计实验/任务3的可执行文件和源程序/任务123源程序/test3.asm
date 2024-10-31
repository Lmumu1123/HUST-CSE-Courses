;PREPEND BEGIN
	.model small
	.stack
	.data
	N dw 0 ;
	result dw 0 ;
	.code
start:
	mov ah, 08h
    int 21h
    sub al, '0'
	xor ah,ah
	mov N, ax
	xor ax,ax
	call ditui1
	mov ax,result
	call print_ax
;******exit******
	mov ax, 4c00h
	int 21h
;******exit******
;PREPEND END
;TEMPLATE BEGIN
ditui1 proc
;ditui1
;***begin****
    mov CX, N       ;注意存储器间不能传值，mov result,N (×)
    mov result, CX	;当N为0/1/2时，结果先进行记录
    cmp N, 2
    je two
	cmp N, 1
    je one
    cmp N, 0
    je zero
    dec N
    call ditui1
	mov dx,ax	    ;暂存f(n-1)
	add ax,bx	    ;bx中存放f(n-2)，进行运算得f(n)
	mov bx,dx	    ;将f(n-1)存在bx，再进入下一次递归
    mov result, ax	;将每次的结果存入result
	ret
two:
	mov ax, 2	    ;ax存此时n-1
	mov bx, 1	    ;bx存此时n-2
	ret
one:
	mov ax, 1
    mov bx, 0
	ret
zero:
	mov ax, 0
    mov bx, 0
    ret
;****end******
ditui1 endp
;TEMPLATE END
;function: print_eax value in decimal;
;input:eax, use buffer (10 bytes)
print_ax proc
    push bx
    push ax
    push cx
    push dx
    xor dx, dx
    xor cx, cx  ;初始化cx,用作计数器
    mov bx, 10  ;用bx让ax每次除以10，余数为每一位的数字
pri_1:
    inc cx      
    div bx      ;将ax除以10，得到的余数即为最低位，最先压进栈
    add dx, 30h ;将余数转化为ascii码
    push dx     ;将余数的ascii码压入栈中
    xor dx, dx  
    cmp ax, 0   ;如果ax除以10后得到0，退出循环
    jnz pri_1
    mov ah, 02h ;准备输出字符
pri_2:
    pop dx      ;取出输入的值
    int 21h
    loop pri_2 

    pop dx
    pop cx
    pop ax
    pop bx
    ret
print_ax endp
;function end
end	start
;//APPEND END
