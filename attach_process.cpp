#include "data_def.h"
#include <stdlib.h>
#include <string.h>

void Attach_Process(char *buf, int len, int *interactive_input_fd)
{
        buf[len] = '\0';
        int to_attach_pid = atoi(buf + strlen("attach_process "));
        (*interactive_input_fd) = pid_to_stdin[to_attach_pid];
}