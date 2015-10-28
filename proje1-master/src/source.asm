;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;;
;;;[BLG-413E][SYSTEM PROGRAMMING]
;;;[PROJECT-1]
;;;[source.asm]
;;;[TO COMPILE][source.c FILE SHOULD BE INSIDE OF THE SAME DIRECTORY!]
;;;[TO COMPILE][nasm -f elf32 source.asm -o asm.o]
;;;[TO COMPILE][gcc -c source.c -o c.o]
;;;[TO COMPILE][gcc c.o asm.o -o source]
;;;[TO RUN][./source {PARAMTERES, in source.c headers!}]
;;;
;;;
;;;[PROJECT MEMBERS]
;;;[040100135] [GÖKBERK GÜLGÜN]
;;;[150110121] [TALHA ÇOLAKOGLU]
;;;[150110119] [CANER ÖZKAYMAK]
;;;
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

segment .text

global sum, scale, _add , mult, itu
global square

sum:
	push ebp            ; save the old base pointer value
    mov  ebp,esp        ; base pointer <- stack pointer
    push ebx
    mov  ebx,[ebp + 8]  ; array
    mov  eax,[ebp + 12] ; array size
    mul  eax			; now it's square            
    mov  ecx, eax       ; tranferred into ecx
    xor  eax, eax		; clearing eax
sloop:    
	dec  ecx			; decrementing index pointer
    add  eax, [ebx + ecx*4]
    cmp  ecx, 0			; if index pointer 0, we are done
    jne  sloop
    pop  ebx
    pop  ebp
    ret

_add:
	push ebp            ; save the old base pointer value
    mov  ebp,esp        ; base pointer <- stack pointer
    push ebx
    push edi
    push esi
    mov  esi,[ebp + 8]  ; array1
    mov  ebx,[ebp + 12] ; array2
    mov  edi,[ebp + 16] ; result
    mov  eax,[ebp + 20] ; size
    mul  eax			;square of size
    mov  ecx, eax       ;ecx counter
aloop:
	dec  ecx
	mov  edx,  [esi + ecx*4] ;first matrix -> edx
	add  edx,  [ebx + ecx*4] ; edx + second matrix -> edx
	mov  [edi + ecx*4], edx  ; edx -> result matrix
	cmp  ecx, 0			; if index pointer 0, we are done
    jne  aloop
    pop  esi
    pop  edi
    pop  ebx
    pop  ebp
    ret
    
scale:
	push ebp
	mov ebp,esp
	push esi
	mov esi,[ebp+8] ;array
	mov ebx,[ebp+12] ;value
	mov edi,[ebp+16] ;result
	mov eax,[ebp+20] ;matrix size
	mul eax ; matrix size
	xor ecx,ecx
	mov ecx,eax 
	xor eax,eax
loopfor:
    dec ecx
	mov eax,[esi + ecx*4] ;load matrix
	mul ebx
	mov [edi + ecx*4],eax  ; load matrix with scale function
	cmp ecx,0
	jne loopfor
	pop esi
	pop ebp
	ret

; defining local variables for mult
%define LOCAL_VAR 20
%define X        dword [ebp - 4]
%define Y        dword [ebp - 8]
%define Z        dword [ebp - 12]
%define subtotal dword [ebp - 16] 
%define size     dword [ebp - 20]
 	
mult:
	push ebp
	mov  ebp,esp
	sub  esp, LOCAL_VAR 		;allocating memory for local variable
	push ebx
	push esi
	push edi
	mov esi,[ebp+8]  			;matrixA
	mov ebx,[ebp+12] 			;matrixB
	mov edi,[ebp+16] 			;result2
	mov edx,[ebp+20] 			;matrixsize
	mov size, edx
	mov X,edx
loopx:
	dec X
	mov edx, size
	mov Y,edx
loopy: 
	dec Y
	xor ecx, ecx 				;ecx subtotal of mult
	mov edx, size
	mov Z,edx
	mov subtotal,0
loopz:
	dec Z
	mov eax, X
	mul size
	add eax, Z    				;now this is pointer of matrix1
	mov ecx, [esi + eax*4] 
	mov eax, Z
	mul size		
	add eax, Y    				;now this is pointer of matrix2
	add edx, [ebx + eax*4] 		
	mov eax, ecx
	mul edx   					;matris1*matris2
	add subtotal, eax
	cmp Z,0
	jne loopz
; end of loopz
	mov eax, X
	mul size
	add eax, Y    				;now this is pointer of result
	mov edx, subtotal
	mov [edi + eax*4], edx
	cmp Y,0
	jne loopy
; end of loopy
	cmp X,0
	jne loopx
; end of loopx
end:
	pop edi
	pop esi
	pop ebx
	add esp, LOCAL_VAR  		;deallocate memory
	pop ebp
	ret

square:
	   push ebp
	   mov  ebp,esp
	   mov ecx,[ebp+8] ;array load
	   mov ebx,[ebp+8] ;array load
	   mov eax,[ebp+12] ;result
	   mov edx,[ebp+16] ;size
	   push edx
	   push eax
	   push ebx
	   push ecx
	   call mult ;call multiplication assembly function
	   add esp,16 ;allocate for pushes
	   mov esp,ebp
	   pop ebp
	   ret

%define lok        dword [ebp - 4]	   
   
itu:
	push ebp
	mov ebp,esp
	sub esp,4
	push edi
	push esi	
	mov  esi,[ebp + 8]  ; array
    mov  eax,[ebp + 12] ; array size
	mov ecx, eax
	mul eax		    ; even array size
	mov edx, ecx
	mov ecx, eax	    ; even array size in ecx
	mov eax, edx        ; old array size in eax
	
	mov edi, edx	    ; old array size is in edi
	xor edx, edx        ; clearing edx
	mov dword ebx, 11
	div ebx           ; dividing eax with 11, quotiant in eax, remainder in edx now
	xor ebx, ebx        ; ebx is zero
	mov edx, eax	    ; quotiant is in edx
	mov lok, edx
allzero:
	mov dword [esi + ebx*4], 0
	inc ebx
	cmp ebx, ecx
	jne allzero
	xor ebx, ebx
	
otherone:
	xor eax,eax
	mov eax,edx
	add eax, edx
	add eax, edx
	add eax, ebx
	mov dword [esi + eax*4],1		;itu's T's left
	add eax, edx
	add eax, edx
	mov dword [esi + eax*4],1		;itu's T's right
	mov eax,ecx
	sub eax, edi
	add eax, edx
	add eax, edx
	add eax, edx
	add eax, edx
	add eax, edx
	add eax, edx
	add eax, edx
	add eax, edx
	add eax, ebx
	mov dword [esi + 4*eax],1	;itu's U's middle
	inc ebx
	cmp ebx, edx
	jne otherone
	xor ecx, ecx		; ecx is now zero
	xor ebx, ebx
allone:
	mov eax,ecx
	mul edi			;for each line eax is line's first number
	add eax, ebx
	add eax, lok		;one edx
	mov dword [esi + 4*eax],1
	add eax, lok
	add eax, lok
	add eax, lok	;four edx is added
	mov dword [esi + 4*eax],1
	add eax, lok
	add eax, lok
	add eax, lok
	mov dword [esi + 4*eax],1
	add eax, lok
	add eax, lok
	mov dword [esi + 4*eax],1
	inc ebx
	cmp ebx, lok
	jne allone
	xor ebx, ebx
	inc ecx
	cmp ecx, edi
	jne allone
	pop esi
	pop edi
	add esp,4 
	pop ebp
	ret


