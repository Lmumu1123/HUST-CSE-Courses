;PREPEND BEGIN
	.model small
	.stack
	.data
	.code
start:
	mov ah, 01h
    int 21h
    sub al, '0'
	xor ah,ah
	push ax	
	call ditui1
	pop ax
    mov bx,ax
	mov dl,0dh
	mov ah,02h
	int 21h
	mov dl,0ah
	mov ah,02h
	int 21h
	mov ax,bx
	call print_ax
;******exit******
    mov ax, 4c00h
    int 21h
;******exit******
;PREPEND END
;TEMPLATE BEGIN   
ditui1 proc
;***begin****
    push ax
    push bx
    push cx
    push dx
    push bp

    mov bp,sp 
    mov ax,[bp+12] ;将N的值从堆栈中取出，存入ax
    cmp  ax,2   ;如果n等于0/1/2，则f(n)的值等于n，可直接结束
    jbe   done   

    ;计算公式为f(n)=f(n-1)+f(n-2)，则分为-1和-2两种去进行递归计算
    ;用栈传递入口和出口参数的形式

    mov dx, ax  ;用dx把ax的值保留下来
    dec ax      ;计算f(n-1)
    push ax     ;将ax压入栈中，用堆栈进行传递
    call ditui1 
    pop ax      ;将值从堆栈中取出
    mov bx, ax  ;将计算结果存入bx

    mov ax, dx  ;将ax值复原
    sub ax, 2   ;计算f(n-2)
    push ax     ;作用同上
    call ditui1
    pop ax
    mov cx, ax  ;将计算结果存入cx

    mov ax, bx  ;计算f(n-1)+f(n-2)
    add ax, cx

done:
    mov [bp+12],ax ;将计算结果通过堆栈返回
    pop bp
    pop dx
    pop cx
    pop bx
    pop ax

    ret
;****end******
;TEMPLATE END
ditui1 endp
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
end	start
;//APPEND END
