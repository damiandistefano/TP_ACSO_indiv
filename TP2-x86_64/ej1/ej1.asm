; === DEFINES ===
%define NULL 0
%define TRUE 1
%define FALSE 0

; Estructura string_proc_list
%define LIST_FIRST 0
%define LIST_LAST 8
%define LIST_SIZE 16

; Estructura string_proc_node
%define NODE_NEXT 0
%define NODE_PREV 8
%define NODE_TYPE 16
%define NODE_HASH 24
%define NODE_SIZE 32

section .data
empty_str: db 0  ; Cadena vacía

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

extern malloc
extern free
extern str_concat


string_proc_list_create_asm:
    push rbp
    mov rbp, rsp

    mov rdi, LIST_SIZE
    call malloc
    test rax, rax
    je .end_create_list

    mov qword [rax + LIST_FIRST], NULL
    mov qword [rax + LIST_LAST], NULL

.end_create_list:
    pop rbp
    ret

string_proc_node_create_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12

    movzx rbx, dil      ; type
    mov r12, rsi        ; hash

    mov rdi, NODE_SIZE
    call malloc
    test rax, rax
    je .end_create_node

    mov qword [rax + NODE_NEXT], NULL
    mov qword [rax + NODE_PREV], NULL
    mov byte [rax + NODE_TYPE], bl
    mov qword [rax + NODE_HASH], r12

.end_create_node:
    pop r12
    pop rbx
    pop rbp
    ret

string_proc_list_add_node_asm:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push rbx
    push r12
    push r13
    push r14

    mov r12, rdi        ; list
    movzx r13, sil      ; type
    mov r14, rdx        ; hash

    mov rdi, r13
    mov rsi, r14
    call string_proc_node_create_asm
    mov rbx, rax        ; new_node

    test rbx, rbx
    jz .ret_null

    test r12, r12
    jz .ret_null

    mov rax, [r12 + LIST_FIRST]
    test rax, rax
    jnz .append_node

    
    mov [r12 + LIST_FIRST], rbx
    mov [r12 + LIST_LAST], rbx
    jmp .done

.append_node:
    mov rax, [r12 + LIST_LAST]
    mov [rbx + NODE_PREV], rax
    mov [rax + NODE_NEXT], rbx
    mov [r12 + LIST_LAST], rbx

.done:
    mov rax, rbx

.ret_null:
    add rsp, 16
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret


string_proc_list_concat_asm:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov r15, rdi        ; list
    movzx r13, sil      ; type
    mov r14, rdx        ; initial_hash

    test r15, r15
    jz .return_copy

    
    mov rdi, empty_str
    mov rsi, r14
    call str_concat
    mov r12, rax        ; result

    mov rbx, [r15 + LIST_FIRST]   ; current

.loop:
    test rbx, rbx
    jz .end_concat

    movzx rax, byte [rbx + NODE_TYPE]
    cmp al, r13b
    jne .skip

    mov rdi, r12
    mov rsi, [rbx + NODE_HASH]
    call str_concat

    mov rdi, r12
    mov r12, rax
    call free

.skip:
    mov rbx, [rbx + NODE_NEXT]
    jmp .loop

.end_concat:
    mov rax, r12
    jmp .restore

.return_copy:
    mov rdi, empty_str
    mov rsi, r14
    call str_concat

.restore:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    add rsp, 16
    pop rbp
    ret
