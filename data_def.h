#ifndef _data_def_
#define _data_def_

#include <unordered_map>
extern int port;
extern char *ip_addr;
extern char *runc_path;
extern std::unordered_map<int, int> pid_to_stdin;
enum CMD : int {
        create_container, start_container, run_container, 
        exec_command, attach_process, container_input, 
        kill_container, pause_container, resume_container, 
        list_container, delete_container, checkpoint_container, 
        restore_container, ps_process, events_look, 
        spec_bundle, update_container, state_container, stop_serve
};
enum Interactive {
        True, False
};
struct Sockfd_Pid_And_Stdin {
        int shim_cli_fd;
        int container_pid;
        int std_input_fd;
};
struct Sockfd_And_Filefd {
        int shim_cli_fd; 
        int file_fd;
        char *str;
};
struct Pid_And_Stdin {
        int new_pid;
        int std_input_fd;
};

#endif 