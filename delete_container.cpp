#include "header.h"
#include "data_def.h"

void Delete_Container(char *buf, int len, int shim_cli_fd)
{
        int pipe_fd[2];
        pipe(pipe_fd);
        char *argv[30];
        argv[0] = "runc";
        int cur_idx = strlen("delete_container ");
        int tmp_idx = cur_idx;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        ++cur_idx;
        int next_idx = 1;
        if (strcmp(buf + tmp_idx, "-gb") == 0) {
                while (true) {
                        tmp_idx = cur_idx;
                        while (cur_idx < len && buf[cur_idx] != ' ') 
                                ++cur_idx;
                        buf[cur_idx] = '\0';
                        ++cur_idx;
                        if (strcmp(buf + tmp_idx, "-ge") == 0) 
                                break;
                        argv[next_idx++] = buf + tmp_idx;
                }
                argv[next_idx++] = "delete";
        } else {
                argv[next_idx++] = "delete";
                argv[next_idx++] = buf + tmp_idx;
        }
        while (cur_idx < len) {
                tmp_idx = cur_idx;
                while (cur_idx < len && buf[cur_idx] != ' ') 
                        ++cur_idx;
                if (cur_idx == len) {
                        buf[cur_idx] = '\0';
                        argv[next_idx++] = buf + tmp_idx;
                        break;
                }
                buf[cur_idx] = '\0';
                argv[next_idx++] = buf + tmp_idx;
                ++cur_idx;
        }
        argv[next_idx++] = (char *)(NULL);
        int pid = fork();
        if (pid == -1)  {
                perror("fork");
                pthread_exit(NULL);
        } else if(pid > 0) {
                close(pipe_fd[1]);
                int runc_exit_code = -1;
                waitpid(pid, &runc_exit_code, 0);
                if (runc_exit_code != 0) {
                        char err_info[1001] = {0};
                        strcpy(err_info, "delete_container_error ");
                        int len = strlen("delete_container_error ");
                        int read_len = read(pipe_fd[0], err_info + len, 1000 - len);
                        err_info[len + read_len] = '\0';
                        write(shim_cli_fd, err_info, len + read_len);
                        close(pipe_fd[0]);
                } else {
                        close(pipe_fd[0]);
                }
        } else if (pid == 0) {
                close(pipe_fd[0]);
                close(2);
                dup(pipe_fd[1]);
                close(pipe_fd[1]);
                execv((const char *)runc_path, (char *const *)argv);
                perror("execv");
                exit(1);
        }
}