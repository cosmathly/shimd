#include "header.h"
#include "data_def.h"

void Ps_Process(char *buf, int len, int shim_cli_fd)
{
        int pipe1_fd[2];
        int pipe2_fd[2];
        pipe(pipe1_fd);
        pipe(pipe2_fd);
        char *argv[30];
        argv[0] = "runc";
        int cur_idx = strlen("ps_process ");
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
                argv[next_idx++] = "ps";
        } else {
                argv[next_idx++] = "ps";
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
        if (pid == -1) {
                perror("fork");
                pthread_exit(NULL);
        } else if (pid > 0) {
                close(pipe1_fd[1]);
                close(pipe2_fd[1]);
                int runc_exit_code = -1;
                waitpid(pid, &runc_exit_code, 0);
                if (runc_exit_code != 0) {
                        char err_info[1001] = {0};
                        strcpy(err_info, "ps_process_error ");
                        int len = strlen("ps_process_error ");
                        int read_len = read(pipe1_fd[0], err_info + len, 1000 - len);
                        err_info[len + read_len] = '\0';
                        write(shim_cli_fd, err_info, len + read_len);
                        close(pipe1_fd[0]);
                        close(pipe2_fd[0]);
                } else {
                        close(pipe1_fd[0]);
                        char ps_info[2001] = {0};
                        strcpy(ps_info, "ps_info ");
                        int len = strlen("ps_info ");
                        int read_len = read(pipe2_fd[0], ps_info + len, 2000 - len);
                        ps_info[len + read_len] = '\0';
                        write(shim_cli_fd, ps_info, len + read_len);
                        close(pipe2_fd[0]);
                }
        } else if (pid == 0) {
                close(pipe1_fd[0]);
                close(pipe2_fd[0]);
                close(2);
                close(1);
                dup(pipe2_fd[1]);
                close(pipe2_fd[1]);
                dup(pipe1_fd[1]);
                close(pipe1_fd[1]);
                execv((const char *)runc_path, (char *const *)argv);
                perror("execv");
                exit(1);
        }
}