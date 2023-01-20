#pragma once

#include <Windows.h>
#include <string>
#include <sstream>
#include <typeinfo>
#include <ctime>

#define MAX_MSG_SIZE 1500
#define WAITING_TIME 10

/*-----------------------Process_handler----------------------*/
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
error_process wait_for_answ(HANDLE* pipe_out_r, int w_time_ms);

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
/*--------------------------------------------------------------*/


/*-----------------------Engine_handler-------------------------*/
typedef enum _engine_state {
	NOT_LOADED,
	WORKING,
} engine_state;

typedef enum _engine_error {
	ENGINE_OK,
	ENGINE_LOAD_FAIL,
	ENGINE_INCORRECT_COMMAND,
	ENGINE_INCORRECT_COMMAND_TYPE,
	ENGINE_TIMEOUT
} engine_error;

typedef enum _engine_command_word {
	_uci,
	_debug, _on, _off,
	_isready,
	_setoption,
	_name,
	_value,
	_register,
	_later,
	_code,
	_ucinewgame,
	_position,
	_fen,
	_startpos,
	_moves,
	_go,
	_searchmoves,
	_ponder,
	_wtime,
	_btime,
	_winc,
	_binc,
	_movestogo,
	_depth,
	_nodes,
	_mate,
	_movetime,
	_infinite,
	_stop,
	_ponderhit,
	_quit,
} engine_command_word;

class UCI_Engine {
public:
	std::wstring path_to_engine{ L"" };     // Full path to engine.exe

	UCI_Engine() {}

	UCI_Engine(std::wstring path_to_engine_in
	) {
		path_to_engine = path_to_engine_in;

		pipe_in_w = NULL;
		pipe_out_r = NULL;
		state = NOT_LOADED;
	}

	/* Call after init_engine_struct. Create new thread and 2 pipes for communication. Run chess_engine.exe. */
	engine_error load();

	/* Closes cless_engine.exe */
	void close();

	/* Send command to engine in the following format (example):
		send_command (
					  _uci,
					  (or) _debug, _on (or _off),
					  (or) _setoption, _name, (int)<id>, (or _value, (int)<x>,)
					  (or) ...(and other from specification of UCI interface)
					 );
	*/
	template<typename Command, typename... Other>
	engine_error send_command(Command word, Other... args);

	/* Busy waits for answer WAITING_TIME secons (or int ms). Returns
	ENGINE_OK, if engine answers,
	ENGINE_TIMEOUT, if time exceeded. */
	engine_error wait_answ();
	engine_error wait_answ(int);

	/* Returns true, if some message from engine is available, else false.*/
	bool check_answ();

	/* Return row answer (string) of engine (max MAX_MSG_SIZE/int bytes). It can return empty string, if engine didn't answer.
	Recommended to call this func after check_answ() returns true (or wait_answ()). */
	std::string get_answ_string();

	/* Returns row answer (one char) of engine.
	Recommended to call this func after check_answ() returns true (or wait_answ()). */
	char get_answ_char();

	/* Returs engine state.
	NOT_LOADED if chess_engine.exe didn't work,
	WORKING if chess_engine.exe simply waits for comands (do nothing),
	THINKING if chess_engine.exe analysing. */
	engine_state get_state();

private:
	HANDLE pipe_in_w{ NULL };       // Used by app to send msg to engine
	HANDLE pipe_out_r{ NULL };       // Used by app to recieve msg from engine
	engine_state state{ NOT_LOADED }; // Current state of engine (working or not and so on)
};

/* Returns std::string command, that was build from words. */
template <typename Command>
std::string get_command_string(Command word)
{
	std::stringstream strm;
	strm << word;
	std::string command = strm.str();

	if (typeid(Command) == typeid(std::string)
		|| typeid(Command) == typeid(const char*) || typeid(Command) == typeid(char*)
		|| typeid(Command) == typeid(int) || typeid(Command) == typeid(float)) {
		return command;
	}
	else
	{
		int command_word = std::stoi(command);
		switch (command_word)
		{
		case _uci:			return	std::string("uci");
		case _debug:		return	std::string("debug");
		case _on:			return	std::string("on");
		case _off:			return	std::string("off");
		case _isready:		return	std::string("isready");
		case _setoption:	return	std::string("setoption");
		case _name:			return	std::string("name");
		case _value:		return	std::string("value");
		case _register:		return	std::string("register");
		case _later:		return	std::string("later");
		case _code:			return	std::string("code");
		case _ucinewgame:	return	std::string("ucinewgame");
		case _position:		return	std::string("position");
		case _fen:			return	std::string("fen");
		case _startpos:		return	std::string("startpos");
		case _moves:		return	std::string("moves");
		case _go:			return	std::string("go");
		case _searchmoves:	return	std::string("searchmoves");
		case _ponder:		return	std::string("ponder");
		case _wtime:		return	std::string("wtime");
		case _btime:		return	std::string("btime");
		case _winc:			return	std::string("winc");
		case _binc:			return	std::string("binc");
		case _movestogo:	return	std::string("movestogo");
		case _depth:		return	std::string("depth");
		case _nodes:		return	std::string("nodes");
		case _mate:			return	std::string("mate");
		case _movetime:		return 	std::string("movetime");
		case _infinite:		return	std::string("infinite");
		case _stop:			return	std::string("stop");
		case _ponderhit:	return	std::string("ponderhit");
		case _quit:			return	std::string("quit");
		default:			return	std::string("INVALID_COMMAND");
		}
	}
}

template <typename Command, typename... Other>
std::string get_command_string(Command word, Other... args)
{
	std::string command = get_command_string(word); command.append(" ");
	return command.append(get_command_string(args...));
}

template<typename Command, typename... Other>
engine_error UCI_Engine::send_command(Command word, Other... args) {
	std::string command = get_command_string(word, args...);
	size_t pos = 0;

	// Add '\n' at the end of the command
	if (command.size() > 0 && command[command.size() - 1] != '\n') command.append("\n");

	// Check string for error
	pos = command.find("INVALID_COMMAND");
	if (pos != std::string::npos) {
		return ENGINE_INCORRECT_COMMAND;
	}

	send_message(&this->pipe_in_w, command.c_str());

	return ENGINE_OK;
}
/*-------------------------------------------------------------*/