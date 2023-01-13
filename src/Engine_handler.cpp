#include "Engine_handler.h"
#include <string.h>
#include <string>
#include <stdarg.h>
#include <typeinfo>

engine_error UCI_Engine::load()
{
	// Creates an engine process (run engine.exe).
	if (create_process(this->path_to_engine, &this->pipe_in_w, &this->pipe_out_r) == PROCESS_CREATE_OK) {
		this->state = WORKING;
		return ENGINE_OK;
	}
	else {
		return ENGINE_LOAD_FAIL;
	}
}

void UCI_Engine::close()
{
	this->send_command(COMMAND_TYPE_PASSTHROUGH, _quit);
	close_stream_handle(&this->pipe_in_w);
	close_stream_handle(&this->pipe_out_r);
	this->pipe_in_w = NULL;
	this->pipe_out_r = NULL;
	this->state = NOT_LOADED;
}

engine_error UCI_Engine::wait_answ()
{
	if (wait_for_answ(&this->pipe_out_r) == PROCESS_OK) return ENGINE_OK;
	else return ENGINE_TIMEOUT;
}

bool UCI_Engine::check_answ()
{
	if (check_pipe(&this->pipe_out_r) != PROCESS_PIPE_NO_MSG) return true;
	else return false;
}

std::string UCI_Engine::get_answ_string()
{
	char answer[MAX_MSG_SIZE] = { 0 };
	recieve_message(&this->pipe_out_r, answer);

	return std::string(answer);
}

char UCI_Engine::get_answ_char()
{
	char answer = '\0';
	recieve_char(&this->pipe_out_r, &answer);

	return answer;
}

engine_state UCI_Engine::get_state()
{
	return this->state;
}
