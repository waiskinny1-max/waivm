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
%define WAI_OP_LOAD_ABS 16
%define WAI_OP_LOAD_REG 17
%define WAI_OP_STORE_ABS 18
%define WAI_OP_STORE_REG 19
%define WAI_OP_PUSH 20
%define WAI_OP_POP 21
%define WAI_OP_CALL 22
%define WAI_OP_RET 23
%define WAI_OP_CMP_IMM 24
%define WAI_OP_CMP_REG 25
%define WAI_OP_JE 26
%define WAI_OP_JNE 27
%define WAI_OP_NOP 28
%define WAI_OP_MOD_IMM 29
%define WAI_OP_MOD_REG 30
%define WAI_OP_AND_IMM 31
%define WAI_OP_AND_REG 32
%define WAI_OP_OR_IMM 33
%define WAI_OP_OR_REG 34
%define WAI_OP_XOR_IMM 35
%define WAI_OP_XOR_REG 36
%define WAI_OP_NOT 37
%define WAI_OP_SHL_IMM 38
%define WAI_OP_SHL_REG 39
%define WAI_OP_SHR_IMM 40
%define WAI_OP_SHR_REG 41

%define WAI_OK 0
%define WAI_ERR_BAD_OPCODE 4
%define WAI_ERR_IP_OUT_OF_BOUNDS 5
%define WAI_ERR_BAD_REGISTER 6
%define WAI_ERR_DIV_ZERO 7
%define WAI_ERR_BAD_JUMP 8
%define WAI_ERR_MEMORY_OOB 10
%define WAI_ERR_STACK_OVERFLOW 11
%define WAI_ERR_STACK_UNDERFLOW 12
%define WAI_ERR_BAD_SHIFT 13

%define VM_REGS 0
%define VM_IP 64
%define VM_ZF 72
%define VM_HALTED 73
%define VM_CODE 80
%define VM_CODE_COUNT 88
%define VM_ERROR 96
%define VM_MEMORY 128
%define VM_SP 65664

%define WAI_MEMORY_SIZE 65536
%define WAI_VALUE_SIZE 8

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
    cmp eax, WAI_OP_LOAD_ABS
    je .op_load_abs
    cmp eax, WAI_OP_LOAD_REG
    je .op_load_reg
    cmp eax, WAI_OP_STORE_ABS
    je .op_store_abs
    cmp eax, WAI_OP_STORE_REG
    je .op_store_reg
    cmp eax, WAI_OP_PUSH
    je .op_push
    cmp eax, WAI_OP_POP
    je .op_pop
    cmp eax, WAI_OP_CALL
    je .op_call
    cmp eax, WAI_OP_RET
    je .op_ret
    cmp eax, WAI_OP_CMP_IMM
    je .op_cmp_imm
    cmp eax, WAI_OP_CMP_REG
    je .op_cmp_reg
    cmp eax, WAI_OP_JE
    je .op_je
    cmp eax, WAI_OP_JNE
    je .op_jne
    cmp eax, WAI_OP_NOP
    je .op_nop
    cmp eax, WAI_OP_MOD_IMM
    je .op_mod_imm
    cmp eax, WAI_OP_MOD_REG
    je .op_mod_reg
    cmp eax, WAI_OP_AND_IMM
    je .op_and_imm
    cmp eax, WAI_OP_AND_REG
    je .op_and_reg
    cmp eax, WAI_OP_OR_IMM
    je .op_or_imm
    cmp eax, WAI_OP_OR_REG
    je .op_or_reg
    cmp eax, WAI_OP_XOR_IMM
    je .op_xor_imm
    cmp eax, WAI_OP_XOR_REG
    je .op_xor_reg
    cmp eax, WAI_OP_NOT
    je .op_not
    cmp eax, WAI_OP_SHL_IMM
    je .op_shl_imm
    cmp eax, WAI_OP_SHL_REG
    je .op_shl_reg
    cmp eax, WAI_OP_SHR_IMM
    je .op_shr_imm
    cmp eax, WAI_OP_SHR_REG
    je .op_shr_reg

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
    movzx rax, byte [r13 + INS_A]
    cmp al, 7
    ja .err_bad_register
    ret

.load_reg_b_index:
    movzx rax, byte [r13 + INS_B]
    cmp al, 7
    ja .err_bad_register
    ret

.check_jump_imm:
    mov rax, [r13 + INS_IMM]
    test rax, rax
    js .err_bad_jump
    mov rbx, [r12 + VM_CODE_COUNT]
    cmp rax, rbx
    jae .err_bad_jump
    ret

.check_jump_rax:
    mov rbx, [r12 + VM_CODE_COUNT]
    cmp rax, rbx
    jae .err_bad_jump
    ret

.check_mem_addr_rbx:
    test rbx, rbx
    js .err_memory_oob
    cmp rbx, WAI_MEMORY_SIZE - WAI_VALUE_SIZE
    ja .err_memory_oob
    ret

.check_shift_rbx:
    test rbx, rbx
    js .err_bad_shift
    cmp rbx, 63
    ja .err_bad_shift
    ret

.push_rax:
    mov rbx, [r12 + VM_SP]
    cmp rbx, WAI_VALUE_SIZE
    jb .err_stack_overflow
    sub rbx, WAI_VALUE_SIZE
    mov [r12 + VM_SP], rbx
    mov [r12 + VM_MEMORY + rbx], rax
    ret

.pop_rax:
    mov rbx, [r12 + VM_SP]
    cmp rbx, WAI_MEMORY_SIZE - WAI_VALUE_SIZE
    ja .err_stack_underflow
    mov rax, [r12 + VM_MEMORY + rbx]
    add rbx, WAI_VALUE_SIZE
    mov [r12 + VM_SP], rbx
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

.op_mod_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rbx, [r13 + INS_IMM]
    test rbx, rbx
    je .err_div_zero
    mov rax, [r12 + VM_REGS + r14 * 8]
    cqo
    idiv rbx
    mov [r12 + VM_REGS + r14 * 8], rdx
    mov rax, rdx
    call .set_zf_from_rax
    jmp .loop

.op_mod_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rbx, [r12 + VM_REGS + rax * 8]
    test rbx, rbx
    je .err_div_zero
    mov rax, [r12 + VM_REGS + r14 * 8]
    cqo
    idiv rbx
    mov [r12 + VM_REGS + r14 * 8], rdx
    mov rax, rdx
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

.op_load_abs:
    call .load_reg_a_index
    mov r14, rax
    mov rbx, [r13 + INS_IMM]
    call .check_mem_addr_rbx
    mov rax, [r12 + VM_MEMORY + rbx]
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_load_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rbx, [r12 + VM_REGS + rax * 8]
    call .check_mem_addr_rbx
    mov rax, [r12 + VM_MEMORY + rbx]
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_store_abs:
    call .load_reg_a_index
    mov r14, rax
    mov rbx, [r13 + INS_IMM]
    call .check_mem_addr_rbx
    mov rax, [r12 + VM_REGS + r14 * 8]
    mov [r12 + VM_MEMORY + rbx], rax
    jmp .loop

.op_store_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rbx, [r12 + VM_REGS + rax * 8]
    call .check_mem_addr_rbx
    mov rax, [r12 + VM_REGS + r14 * 8]
    mov [r12 + VM_MEMORY + rbx], rax
    jmp .loop

.op_push:
    call .load_reg_a_index
    mov rax, [r12 + VM_REGS + rax * 8]
    call .push_rax
    jmp .loop

.op_pop:
    call .load_reg_a_index
    mov r14, rax
    call .pop_rax
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_call:
    call .check_jump_imm
    mov r15, rax                ; target
    mov rax, [r12 + VM_IP]      ; return address after call
    call .push_rax
    mov [r12 + VM_IP], r15
    jmp .loop

.op_ret:
    call .pop_rax
    call .check_jump_rax
    mov [r12 + VM_IP], rax
    jmp .loop

.op_cmp_imm:
    call .load_reg_a_index
    mov rdx, [r12 + VM_REGS + rax * 8]
    cmp rdx, [r13 + INS_IMM]
    sete byte [r12 + VM_ZF]
    jmp .loop

.op_cmp_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rdx, [r12 + VM_REGS + r14 * 8]
    cmp rdx, [r12 + VM_REGS + rax * 8]
    sete byte [r12 + VM_ZF]
    jmp .loop

.op_je:
    cmp byte [r12 + VM_ZF], 0
    je .loop
    call .check_jump_imm
    mov [r12 + VM_IP], rax
    jmp .loop

.op_jne:
    cmp byte [r12 + VM_ZF], 0
    jne .loop
    call .check_jump_imm
    mov [r12 + VM_IP], rax
    jmp .loop

.op_nop:
    jmp .loop

.op_and_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rax, [r12 + VM_REGS + r14 * 8]
    and rax, [r13 + INS_IMM]
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_and_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov r15, rax
    mov rax, [r12 + VM_REGS + r14 * 8]
    and rax, [r12 + VM_REGS + r15 * 8]
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_or_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rax, [r12 + VM_REGS + r14 * 8]
    or rax, [r13 + INS_IMM]
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_or_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov r15, rax
    mov rax, [r12 + VM_REGS + r14 * 8]
    or rax, [r12 + VM_REGS + r15 * 8]
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_xor_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rax, [r12 + VM_REGS + r14 * 8]
    xor rax, [r13 + INS_IMM]
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_xor_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov r15, rax
    mov rax, [r12 + VM_REGS + r14 * 8]
    xor rax, [r12 + VM_REGS + r15 * 8]
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_not:
    call .load_reg_a_index
    mov r14, rax
    mov rax, [r12 + VM_REGS + r14 * 8]
    not rax
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_shl_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rbx, [r13 + INS_IMM]
    call .check_shift_rbx
    mov cl, bl
    mov rax, [r12 + VM_REGS + r14 * 8]
    shl rax, cl
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_shl_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rbx, [r12 + VM_REGS + rax * 8]
    call .check_shift_rbx
    mov cl, bl
    mov rax, [r12 + VM_REGS + r14 * 8]
    shl rax, cl
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_shr_imm:
    call .load_reg_a_index
    mov r14, rax
    mov rbx, [r13 + INS_IMM]
    call .check_shift_rbx
    mov cl, bl
    mov rax, [r12 + VM_REGS + r14 * 8]
    shr rax, cl
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.op_shr_reg:
    call .load_reg_a_index
    mov r14, rax
    call .load_reg_b_index
    mov rbx, [r12 + VM_REGS + rax * 8]
    call .check_shift_rbx
    mov cl, bl
    mov rax, [r12 + VM_REGS + r14 * 8]
    shr rax, cl
    mov [r12 + VM_REGS + r14 * 8], rax
    call .set_zf_from_rax
    jmp .loop

.err_bad_opcode:
    mov dword [r12 + VM_ERROR], WAI_ERR_BAD_OPCODE
    jmp .error_return
.err_ip:
    mov dword [r12 + VM_ERROR], WAI_ERR_IP_OUT_OF_BOUNDS
    jmp .error_return
.err_bad_register:
    add rsp, 8                  ; discard helper return address
    mov dword [r12 + VM_ERROR], WAI_ERR_BAD_REGISTER
    jmp .error_return
.err_div_zero:
    mov dword [r12 + VM_ERROR], WAI_ERR_DIV_ZERO
    jmp .error_return
.err_bad_jump:
    add rsp, 8                  ; discard helper return address
    mov dword [r12 + VM_ERROR], WAI_ERR_BAD_JUMP
    jmp .error_return
.err_memory_oob:
    add rsp, 8                  ; discard helper return address
    mov dword [r12 + VM_ERROR], WAI_ERR_MEMORY_OOB
    jmp .error_return
.err_stack_overflow:
    add rsp, 8                  ; discard helper return address
    mov dword [r12 + VM_ERROR], WAI_ERR_STACK_OVERFLOW
    jmp .error_return
.err_stack_underflow:
    add rsp, 8                  ; discard helper return address
    mov dword [r12 + VM_ERROR], WAI_ERR_STACK_UNDERFLOW
    jmp .error_return
.err_bad_shift:
    add rsp, 8                  ; discard helper return address
    mov dword [r12 + VM_ERROR], WAI_ERR_BAD_SHIFT
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
