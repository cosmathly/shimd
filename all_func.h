#ifndef _all_func_
#define _all_func_

/* 
 * The functions that shimd provides are as follows 
 * --------------------------------------------------
 * create_container, start_container, run_container, \
 * --------------------------------------------------
 * exec_command, attach_process, container_input, \
 * --------------------------------------------------
 * kill_container, pause_container, resume_container, \ 
 * --------------------------------------------------
 * list_container, delete_container, checkpoint_container, \ 
 * --------------------------------------------------
 * restore_container, ps_process, events_look, spec_bundle, \
 * --------------------------------------------------
 * update_container, state_container, stop_serve \
 */

/* The following functions realize the above functions */
extern void Create_Container(char *, int, int, int *, int *);
extern void Create_Interactive_Container(char *, int, int, char *, char *, int, int *, int *);
extern void Create_Non_Interactive_Container(char *, int, int, char *, char *, int, int *, int *);
extern void Start_Container(char *, int, int, int, int *);
extern void Run_Container(char *, int, int, int *, int *);
extern void Run_Interactive_Container(char *, int, int, char *, char *, int, int *, int *);
extern void Run_Non_Interactive_Container(char *, int, int, char *, char *, int, int *, int *);
extern void Exec_Command(char *, int, int);
extern void Exec_Interactive_Command(char *, int, int, int);
extern void Exec_Non_Interactive_Command(char *, int, int, int);
extern void Attach_Process(char *, int, int *);
extern void Container_Input(char *, int, int, int);
extern void Kill_Container(char *, int, int);
extern void Pause_Container(char *, int, int);
extern void Resume_Container(char *, int, int);
extern void List_Container(char *, int, int);
extern void Delete_Container(char *, int, int);
extern void Checkpoint_Container(char *, int, int);
extern void Restore_Container(char *, int, int);
extern void Ps_Process(char *, int, int);
extern void Events_Look(char *, int, int);
extern void Spec_Bundle(char *, int, int);
extern void Update_Container(char *, int, int);
extern void State_Container(char *, int, int);
extern void Stop_Serve(int, bool *);

/* other functions */ 
enum CMD : int; 
extern void set_port_reuse(int);
extern CMD parse_cmd(char *, int);
extern void *new_waiter(void *);
extern void *waiter(void *);
extern void *transfer_data(void *);
extern void *serve_shim(void *);
extern void listener();
extern void ignore_sigpipe();

#endif