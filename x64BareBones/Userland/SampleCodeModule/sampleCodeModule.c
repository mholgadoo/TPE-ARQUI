#include "syscalls.h"
#include <stdint.h>
#include "syscalls.h"
#include "shell.h"


#define SYS_READ 1
#define USERNAME_MAX_LEN 32

int main() {
    shell_run("M&M");
    return 0;
}
