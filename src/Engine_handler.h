#pragma once

#define OPTION_MULTI_PV_DEF 4 // Default count of lines to analysis
#define MOVE_TIME_DEFUALT 1 // Default time of analysing
#define OPTION_HASH_DEF 32
#define OPTION_THREAD_DEF 1
#define MAX_MOVE_SIZE 10
#define MAX_WORD_SIZE 100

#include "Process_handler.h"

enum _engine_state {
	NOT_LOADED,
	WORKING,
	THINKING
};

typedef enum _engine_error {
	ENGINE_LOAD_OK,
	ENGINE_LOAD_FAIL
} engine_error;

typedef enum _engine_command_type {
	COMMAND_TYPE_PASSTHROUGH, // Send command without response
	COMMAND_TYPE_GET_ANSW     // Send command and wait for answer
} engine_command_type;

typedef enum _engine_command {
	UCI,
	DEBUG_ON,
	DEBUG_OFF,
	IS_READY,
	UCI_NEW_GAME,
	STOP_ANALYZE,
	QUIT_ENGINE
} engine_command;

typedef enum _engine_answers {
	UCI_OK,
	READY_OK,
	NORMAL_ANSW,
	NO_ANSW
} engine_answers;

typedef struct _engine
{
	LPCWSTR path_to_engine;         // Full path to engine.exe
	HANDLE pipe_in_w;               // Used by app to send msg to engine
	HANDLE pipe_out_r;              // Used by app to recieve msg from engine
	enum _engine_state state;       // Current state of engine (working or not and so on)
	int cpu;
	int multi_pv;
	int hash;
	int move_time;
} engine;

typedef struct _engine_line {
	char move[MAX_MOVE_SIZE];
} engine_line;

/* Initialize all fields with NULL and copy path to field path_to_engine[]. */
void init_engine_struct(engine*, LPCWSTR path_to_engine);

/* Call after init_engine_struct. Create new thread and 2 pipes for communication. Run chess_engine.exe. */
engine_error engine_load(engine* ptr_to_engine);

/* Closes engine. */
void engine_close(engine* ptr_to_engine);

/* Send a simple command to engine. If command type set as COMMAND_TYPE_PASSTHROUGH, than buff_answ should be set as NULL. */
void engine_send_command(engine*, engine_command_type, engine_command, char* buff_answ);

/* Send a custom message to engine. */
void engine_send_command_custom(engine*, engine_command_type, char* msg, char* buff_answ);

/* Send to engine command "go", that force to start thinking. End of analyze after move_time seconds. */
void engine_send_command_start_analyze(engine*, int move_time);

/* Set default options for engine and send them to engine process. */
void engine_set_def_options(engine*);   // Number of lines with analisys

/* Set internal position in engine to analisys, in FEN notation. */
void engine_set_position(engine*, char* fen_string);

/* Recieve all output from engine when it is analising, extract from that input first move of each line.*/
void engine_parse_analisys_output(engine*, engine_line*);

/* Look at answer and return one of the engine_answers*/
engine_answers engine_identify_answer(char* answ);


