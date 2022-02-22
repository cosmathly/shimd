#include <unistd.h>

void Stop_Serve(int shim_cli_fd, bool *break_flag)
{
        close(shim_cli_fd);
        (*break_flag) = false;
}