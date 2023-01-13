#include <Windows.h>
#include <string>

#define MAX_MSG_SIZE 1500
#define WAITING_TIME 10

typedef enum _error_process {
	PROCESS_OK,
	PROCESS_CREATE_OK,
	PROCESS_CREATE_FAIL,
	PROCESS_PIPE_MSG_AVAILABLE,
	PROCESS_PIPE_NO_MSG,
	PROCESS_TIMEOUT
} error_process;

/* Creates standart process for engine */
error_process create_process(std::wstring path, HANDLE* pipe_in_w, HANDLE* pipe_out_r);

/* Closes stream description*/
void close_stream_handle(HANDLE* stream);

/* Block curr process untill child process answers*/
error_process wait_for_answ(HANDLE* pipe_out_r);

/* Send message to pipe */
void send_message(HANDLE* pipe_in_w, const char msg[MAX_MSG_SIZE]);

/* Recieve message from pipe. Msg places in buff */
void recieve_message(HANDLE* pipe_out_r, char* buff);

/* Recieve a char from pipe. Char places in a c_buff. */
void recieve_char(HANDLE* pipe_out_r, char* c_buff);

/* Returns PROCESS_PIPE_MSG_AVAILABLE if pipe is note empty, overwise PROCESS_PIPE_NO_MSG. */
error_process check_pipe(HANDLE* pipe_out_r);

/* Recieve all massages from pipe (clean pipe). */
void clean_pipe_out(HANDLE* pipe_out_r);