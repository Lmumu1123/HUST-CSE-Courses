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
	push ax	
	call ditui1
	pop ax
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
	push ax	;过程中使用了AX、CX和DX
	push cx
	push dx
	push ax	;暂存ax
	mov dl,al	;转换al的高4位
	mov cl,4
	shr dl,cl
	or dl,30h	;al高4位变成3
	cmp dl,39h
	jbe aldisp1
	add dl,7	;是0Ah～0Fh，还要加上7
aldisp1:	mov ah,2	;显示
	int 21h
	pop dx	;恢复原ax值到dx
	and dl,0fh	;转换al的低4位
	or dl,30h
	cmp dl,39h
	jbe aldisp2
	add dl,7
aldisp2:	mov ah,2	;显示
	int 21h
	pop dx
	pop cx
	pop ax
	ret	;过程返回
print_ax endp
end	start
;//APPEND END
