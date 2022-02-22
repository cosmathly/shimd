#include "header.h"
#include "data_def.h"

void Update_Container(char *buf, int len, int shim_cli_fd)
{
        int pipe1_fd[2];
        int pipe2_fd[2];
        pipe(pipe1_fd);
        pipe(pipe2_fd);
        bool if_std_input = false;
        char *argv[30];
        argv[0] = "runc";
        int cur_idx = strlen("update_container ");
        int tmp_idx = cur_idx;
        while (cur_idx < len && buf[cur_idx] != ' ') 
                ++cur_idx;
        buf[cur_idx] = '\0';
        ++cur_idx;
        int next_idx = 1;
        int r_idx = -1;
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
                argv[next_idx++] = "update";
                r_idx = next_idx;
        } else {
                argv[next_idx++] = "update";
                r_idx = next_idx;
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
        if (strcmp(argv[r_idx], "--resources") == 0 
            || strcmp(argv[r_idx], "-r") == 0) {
                if (argv[r_idx+1] == (char *)NULL) {
                        write(shim_cli_fd, "parameter_too_short ", strlen("parameter_too_short "));
                        return ;
                }
                if (strcmp(argv[r_idx+1], "-") == 0) 
                        if_std_input = true;
        }
        int pid = fork();
        if (pid == -1) {
                perror("fork");
                pthread_exit(NULL);
        } else if (pid > 0) {
                if (if_std_input == true) {
                        close(pipe1_fd[1]);
                        close(pipe2_fd[0]);
                        char update_info[1001] = {0};
                        int read_len = read(shim_cli_fd, update_info, 1000);
                        int tmp_idx = 0;
                        while (tmp_idx < read_len && update_info[tmp_idx] != ' ') 
                                ++tmp_idx;
                        update_info[tmp_idx] = '\0';
                        ++tmp_idx;
                        if (strcmp(update_info, "update_info") == 0) {
                                write(pipe2_fd[1], update_info + tmp_idx, read_len - tmp_idx);
                                int runc_exit_code = -1;
                                waitpid(pid, &runc_exit_code, 0);
                                if (runc_exit_code != 0) {
                                        char err_info[1001] = {0};
                                        strcpy(err_info, "update_container_error ");
                                        int len = strlen("update_container_error ");
                                        int read_len = read(pipe1_fd[0], err_info + len, 1000 - len);
                                        err_info[len + read_len] = '\0';
                                        write(shim_cli_fd, err_info, len + read_len);
                                }
                                close(pipe1_fd[0]);
                                close(pipe2_fd[1]);
                        }
                } else {
                        close(pipe1_fd[1]);
                        close(pipe2_fd[0]);
                        close(pipe2_fd[1]);
                        int runc_exit_code = -1;
                        waitpid(pid, &runc_exit_code, 0);
                        if (runc_exit_code != 0) {
                                char err_info[1001] = {0};
                                strcpy(err_info, "update_container_error ");
                                int len = strlen("update_container_error ");
                                int read_len = read(pipe1_fd[0], err_info + len, 1000 - len);
                                err_info[len + read_len] = '\0';
                                write(shim_cli_fd, err_info, len + read_len);
                        }
                        close(pipe1_fd[0]);
                }
        } else if (pid == 0) {
                close(pipe1_fd[0]);
                close(pipe2_fd[1]);
                if (if_std_input == false) {
                        close(pipe2_fd[0]);
                } else {
                        close(0);
                        dup(pipe2_fd[0]);
                        close(pipe2_fd[0]);
                }
                close(2);
                dup(pipe1_fd[1]);
                close(pipe1_fd[1]);
                execv((const char *)runc_path, (char *const *)argv);
                perror("execv");
                exit(1);
        }
}