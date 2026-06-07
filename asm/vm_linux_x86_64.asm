; waivm x86-64 Linux execution core
; System V ABI: int wai_vm_exec_asm(wai_vm *vm)

BITS 64

%define WAI_OP_MOV_IMM 1
%define WAI_OP_MOV_REG 2
%define WAI_OP_ADD_IMM 3
%define WAI_OP_ADD_REG 4
%define WAI_OP_SUB_IMM 5
%define WAI_OP_SUB_REG 6
%define WAI_OP_MUL_IMM 7
%define WAI_OP_MUL_REG 8
%define WAI_OP_DIV_IMM 9
%define WAI_OP_DIV_REG 10
%define WAI_OP_JMP 11
%define WAI_OP_JZ 12
%define WAI_OP_JNZ 13
%define WAI_OP_PRINT 14
%define WAI_OP_HALT 15

%define WAI_OK 0
%define WAI_ERR_BAD_OPCODE 4
%define WAI_ERR_IP_OUT_OF_BOUNDS 5
%define WAI_ERR_BAD_REGISTER 6
%define WAI_ERR_DIV_ZERO 7
%define WAI_ERR_BAD_JUMP 8

%define VM_REGS 0
%define VM_IP 64
%define VM_ZF 72
%define VM_HALTED 73
%define VM_CODE 80
%define VM_CODE_COUNT 88
%define VM_ERROR 96

%define INS_OPCODE 0
%define INS_A 1
%define INS_B 2
%define INS_IMM 4
%define INS_SIZE 12

section .text
global wai_vm_exec_asm
extern wai_vm_emit_print

wai_vm_exec_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov r12, rdi                ; r12 = vm*

.loop:
    cmp byte [r12 + VM_HALTED], 0
    jne .ok_return

    mov rax, [r12 + VM_IP]
    mov rbx, [r12 + VM_CODE_COUNT]
    cmp rax, rbx
    jae .err_ip

    mov r13, [r12 + VM_CODE]
    lea r14, [rax + rax * 2]    ; ip * 3
    shl r14, 2                  ; ip * 12
    add r13, r14                ; r13 = &code[ip]

    inc rax                     ; default next ip
    mov [r12 + VM_IP], rax

    movzx eax, byte [r13 + INS_OPCODE]

    cmp eax, WAI_OP_MOV_IMM
    je .op_mov_imm
    cmp eax, WAI_OP_MOV_REG
    je .op_mov_reg
    cmp eax, WAI_OP_ADD_IMM
    je .op_add_imm
    cmp eax, WAI_OP_ADD_REG
    je .op_add_reg
    cmp eax, WAI_OP_SUB_IMM
    je .op_sub_imm
    cmp eax, WAI_OP_SUB_REG
    je .op_sub_reg
    cmp eax, WAI_OP_MUL_IMM
    je .op_mul_imm
    cmp eax, WAI_OP_MUL_REG
    je .op_mul_reg
    cmp eax, WAI_OP_DIV_IMM
    je .op_div_imm
    cmp eax, WAI_OP_DIV_REG
    je .op_div_reg
    cmp eax, WAI_OP_JMP
    je .op_jmp
    cmp eax, WAI_OP_JZ
    je .op_jz
    cmp eax, WAI_OP_JNZ
    je .op_jnz
    cmp eax, WAI_OP_PRINT
    je .op_print
    cmp eax, WAI_OP_HALT
    je .op_halt

    jmp .err_bad_opcode

; ----- helpers -----
; Validate register in al. Leaves zero-extended register in rax.
.check_reg_al:
    cmp al, 7
    ja .err_bad_register
    movzx rax, al
    ret

.set_zf_from_rax:
    test rax, rax
    sete byte [r12 + VM_ZF]
    ret

.load_reg_a_index:
    mov al, [r13 + INS_A]
    call .check_reg_al
    ret

.load_reg_b_index:
    mov al, [r13 + INS_B]
    call .check_reg_al
    ret

.check_jump_imm:
    mov rax, [r13 + INS_IMM]
    test rax, rax
    js .err_bad_jump
    mov rbx, [r12 + VM_CODE_COUNT]
    cmp rax, rbx
    jae .err_bad_jump
    ret

; ----- instructions -----
.op_mov_imm:
    call .load_reg_a_index
    mov rdx, [r13 + INS_IMM]
    mov [r12 + VM_REGS + rax * 8], rdx
    mov rax, rdx
    call .set_zf_from_rax
    jmp .loop

.op_mov_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rdx, [r12 + VM_REGS + rax * 8]
    mov [r12 + VM_REGS + r14 * 8], rdx
    mov rax, rdx
    call .set_zf_from_rax
    jmp .loop

.op_add_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rdx, [r12 + VM_REGS + r14 * 8]
    add rdx, [r13 + INS_IMM]
    mov [r12 + VM_REGS + r14 * 8], rdx
    mov rax, rdx
    call .set_zf_from_rax
    jmp .loop

.op_add_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rdx, [r12 + VM_REGS + r14 * 8]
    add rdx, [r12 + VM_REGS + rax * 8]
    mov [r12 + VM_REGS + r14 * 8], rdx
    mov rax, rdx
    call .set_zf_from_rax
    jmp .loop

.op_sub_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rdx, [r12 + VM_REGS + r14 * 8]
    sub rdx, [r13 + INS_IMM]
    mov [r12 + VM_REGS + r14 * 8], rdx
    mov rax, rdx
    call .set_zf_from_rax
    jmp .loop

.op_sub_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rdx, [r12 + VM_REGS + r14 * 8]
    sub rdx, [r12 + VM_REGS + rax * 8]
    mov [r12 + VM_REGS + r14 * 8], rdx
    mov rax, rdx
    call .set_zf_from_rax
    jmp .loop

.op_mul_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rax, [r12 + VM_REGS + r14 * 8]
    imul rax, [r13 + INS_IMM]
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_mul_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rdx, [r12 + VM_REGS + rax * 8]
    mov rax, [r12 + VM_REGS + r14 * 8]
    imul rax, rdx
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_div_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rbx, [r13 + INS_IMM]
    test rbx, rbx
    je .err_div_zero
    mov rax, [r12 + VM_REGS + r14 * 8]
    cqo
    idiv rbx
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_div_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rbx, [r12 + VM_REGS + rax * 8]
    test rbx, rbx
    je .err_div_zero
    mov rax, [r12 + VM_REGS + r14 * 8]
    cqo
    idiv rbx
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_jmp:
    call .check_jump_imm
    mov [r12 + VM_IP], rax
    jmp .loop

.op_jz:
    call .load_reg_a_index
    cmp qword [r12 + VM_REGS + rax * 8], 0
    jne .loop
    call .check_jump_imm
    mov [r12 + VM_IP], rax
    jmp .loop

.op_jnz:
    call .load_reg_a_index
    cmp qword [r12 + VM_REGS + rax * 8], 0
    je .loop
    call .check_jump_imm
    mov [r12 + VM_IP], rax
    jmp .loop

.op_print:
    call .load_reg_a_index
    mov rsi, [r12 + VM_REGS + rax * 8]
    mov rdi, r12
    sub rsp, 8                  ; align stack before external C call
    call wai_vm_emit_print
    add rsp, 8
    jmp .loop

.op_halt:
    mov byte [r12 + VM_HALTED], 1
    mov dword [r12 + VM_ERROR], WAI_OK
    jmp .ok_return

.err_bad_opcode:
    mov dword [r12 + VM_ERROR], WAI_ERR_BAD_OPCODE
    jmp .error_return
.err_ip:
    mov dword [r12 + VM_ERROR], WAI_ERR_IP_OUT_OF_BOUNDS
    jmp .error_return
.err_bad_register:
    mov dword [r12 + VM_ERROR], WAI_ERR_BAD_REGISTER
    jmp .error_return
.err_div_zero:
    mov dword [r12 + VM_ERROR], WAI_ERR_DIV_ZERO
    jmp .error_return
.err_bad_jump:
    mov dword [r12 + VM_ERROR], WAI_ERR_BAD_JUMP
    jmp .error_return

.ok_return:
    xor eax, eax
    jmp .finish

.error_return:
    mov eax, 1

.finish:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
