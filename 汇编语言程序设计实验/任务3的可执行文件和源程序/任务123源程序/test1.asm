;PREPEND BEGIN
	.model small
	.stack
	.data
	.code
start:
	mov ah, 08h
    int 21h
    sub al, '0'
	xor ah,ah
	call ditui1
	call print_ax
;******exit******
    mov ax, 4c00h
    int 21h
;******exit******
;PREPEND END
;TEMPLATE BEGIN
;ax=n 寄存器ax传递参数   
ditui1 proc
;ditui1
;***begin****
    push bx
    push cx
    push dx

    cmp  ax,2   ;如果n等于0/1/2，则f(n)的值等于n，可直接结束
    jbe   done   

    ;计算公式为f(n)=f(n-1)+f(n-2)，则分为-1和-2两种去进行递归计算
    ;同时将ax传递参数

    mov dx, ax  ;用dx把ax的值保留下来
    dec ax      ;计算f(n-1)
    call ditui1 
    mov bx, ax  ;将计算结果存入bx

    mov ax, dx  ;将ax值复原
    sub ax, 2   ;计算f(n-2)
    call ditui1
    mov cx, ax  ;将计算结果存入cx

    mov ax, bx  ;计算f(n-1)+f(n-2)
    add ax, cx

done:
    pop dx
    pop cx
    pop bx

    ret
;****end******
ditui1 endp
;TEMPLATE END
;APPEND BEGIN
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
again2:
    pop dx      ;取出输入的值
    int 21h
    loop again2 

    pop dx
    pop cx
    pop ax
    pop bx
    ret
print_ax endp
end start
;//APPEND END
