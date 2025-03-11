/* entry_point.c */

extern int main(void);

/* 
 * _start is the actual entry point of the program.
 * It calls main() and then exits using the Linux sys_exit syscall.
 */
void _start(void) {
    long ret = (long)main();
    __asm__ (
        "mov $60, %%rax\n"   /* sys_exit syscall number on x86_64 */
        "movq %0, %%rdi\n"   /* move exit code (as 64-bit) into rdi */
        "syscall\n"
        : 
        : "r"(ret)
        : "rax", "rdi"
    );
    while (1) {}  /* Should never get here */
}

