/* dummy_stdio.c */

/* Minimal FILE type definition */
typedef struct {
    int dummy;
} FILE;

/* Provide a dummy stderr symbol.
   Here stderr is simply defined as a null pointer.
   The inline assembler directive assigns the required GLIBC version.
*/
FILE *stderr = 0;
__asm__(".symver stderr,stderr@GLIBC_2.2.5");

/* A simple helper function to compute the length of a string */
static unsigned long my_strlen(const char *s) {
    unsigned long len = 0;
    while (s[len])
        len++;
    return len;
}

long unsigned int strlen(const char *str)
{
        const char *s;

        for (s = str; *s; ++s)
                ;
        return (s - str);
}
/* Minimal implementation of fputs.
   This implementation ignores the provided stream and writes the string
   to file descriptor 2 (stderr) using the write system call.
*/
int fputs(const char *str, FILE *stream) {
    (void)stream;  /* ignore the stream parameter */
    unsigned long len = my_strlen(str);
    long ret;
    __asm__ (
        "mov $1, %%rax\n"    /* syscall number for write (1) */
        "mov $2, %%rdi\n"    /* file descriptor 2 (stderr) */
        "mov %1, %%rsi\n"    /* pointer to the string */
        "mov %2, %%rdx\n"    /* length of the string */
        "syscall\n"
        : "=a"(ret)
        : "r"(str), "r"(len)
        : "rdi", "rsi", "rdx"
    );
    return (int)ret;
}

__asm__(".symver fputs, fputs@GLIBC_2.2.5");

