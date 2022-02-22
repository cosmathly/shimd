#include <string.h>
#include <unistd.h>

void Container_Input(char *buf, int len, int shim_cli_fd, int input_fd)
{
        int str_len = strlen("container_input ");
        write(input_fd, buf + str_len, len - str_len);
}