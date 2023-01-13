#include "Engine_handler.h"
#include <string.h>
#include <string>

void init_engine_struct(engine* engn, LPCWSTR path_to_engine) {
	engn->path_to_engine = path_to_engine;
	engn->pipe_in_w = NULL;
	engn->pipe_out_r = NULL;
	engn->state = NOT_LOADED;
	engn->cpu = OPTION_THREAD_DEF;
	engn->hash = OPTION_HASH_DEF;
	engn->multi_pv = OPTION_MULTI_PV_DEF;
	engn->move_time = MOVE_TIME_DEFUALT;
}

engine_error engine_load(engine* engn) {
	// Creates an engine process (run engine.exe).
	if (create_process(engn->path_to_engine, &engn->pipe_in_w, &engn->pipe_out_r) == PROCESS_CREATE_OK) {
		char answ[MAX_MSG_SIZE] = { '\0' };

		// Wait for initializing of engine (synchronize engine and app) (wait answ)
		wait_for_answ(&engn->pipe_out_r);
		clean_pipe_out(&engn->pipe_out_r);

		// Send command "uci" as specificated in uci protocol docs.
		engine_send_command(engn, COMMAND_TYPE_GET_ANSW, UCI, answ);
		if (engine_identify_answer(answ) != UCI_OK) {
			return ENGINE_LOAD_FAIL;
		}
		
		// Send command "ucinewgame" to finally init engine.
		engine_send_command(engn, COMMAND_TYPE_PASSTHROUGH, UCI_NEW_GAME, NULL);

		// Send command "setoption" to setup params of engine.
		engine_set_def_options(engn);

		engn->state = WORKING;
		return ENGINE_LOAD_OK;
	}
	else {
		return ENGINE_LOAD_FAIL;
	}
}

void engine_close(engine* engn) {
	engine_send_command(engn, COMMAND_TYPE_PASSTHROUGH, QUIT_ENGINE, NULL);
	close_stream_handle(&engn->pipe_in_w);
	close_stream_handle(&engn->pipe_out_r);
	engn->state = NOT_LOADED;
}

void engine_send_command(engine* engn, engine_command_type command_type, engine_command command, char* buff_answ) {
	char msg[MAX_MSG_SIZE] = { '\0' };

	switch (command)
	{
	case UCI:
		memcpy(msg, "uci\n", sizeof(char) * 5);
		break;
	case DEBUG_ON:
		memcpy(msg, "debug on\n", sizeof(char) * 10);
		break;
	case DEBUG_OFF:
		memcpy(msg, "debug off\n", sizeof(char) * 11);
		break;
	case IS_READY:
		memcpy(msg, "isready\n", sizeof(char) * 9);
		break;
	case UCI_NEW_GAME:
		memcpy(msg, "ucinewgame\n", sizeof(char) * 12);
		break;
	case STOP_ANALYZE:
		memcpy(msg, "stop\n", sizeof(char) * 6);
		break;
	case QUIT_ENGINE:
		memcpy(msg, "quit\n", sizeof(char) * 6);
		break;
	default:
		break;
	}

	switch (command_type)
	{
	case COMMAND_TYPE_PASSTHROUGH:
		send_message(&engn->pipe_in_w, msg);
		break;
	case COMMAND_TYPE_GET_ANSW:
		send_message(&engn->pipe_in_w, msg);
		wait_for_answ(&engn->pipe_out_r);
		recieve_message(&engn->pipe_out_r, buff_answ);
		break;
	default:
		break;
	}
}

void engine_send_command_custom(engine* engn, engine_command_type command_type, char* msg, char* buff_answ) {
	switch (command_type)
	{
	case COMMAND_TYPE_PASSTHROUGH:
		send_message(&engn->pipe_in_w, msg);
		break;
	case COMMAND_TYPE_GET_ANSW:
		send_message(&engn->pipe_in_w, msg);
		wait_for_answ(&engn->pipe_out_r);
		recieve_message(&engn->pipe_out_r, buff_answ);
		break;
	default:
		break;
	}
}

void engine_send_command_start_analyze(engine* engn, int move_time) {
	std::string buff;

	buff = "go movetime ";
	buff += std::to_string(move_time * 1000);
	buff += "\n";

	engine_send_command_custom(engn, COMMAND_TYPE_PASSTHROUGH, (char*)buff.c_str(), NULL);
}

void engine_set_def_options(engine* engn) {
	std::string buff;

	buff = "setoption name MultiPV value ";
	buff += std::to_string(OPTION_MULTI_PV_DEF);
	buff += "\n";
	engine_send_command_custom(engn, COMMAND_TYPE_PASSTHROUGH, (char*)buff.c_str(), NULL);

	buff = "setoption name Hash value ";
	buff += std::to_string(OPTION_HASH_DEF);
	buff += "\n";
	engine_send_command_custom(engn, COMMAND_TYPE_PASSTHROUGH, (char*)buff.c_str(), NULL);

	buff = "setoption name Threads value ";
	buff += std::to_string(OPTION_THREAD_DEF);
	buff += "\n";
	engine_send_command_custom(engn, COMMAND_TYPE_PASSTHROUGH, (char*)buff.c_str(), NULL);
}

void engine_set_position(engine* engn, char* fen_string) {
	std::string buff;
	char answ[MAX_MSG_SIZE];

	buff = "position fen ";
	buff += fen_string;
	buff += "\n";

	engine_send_command_custom(engn, COMMAND_TYPE_PASSTHROUGH, (char*)buff.c_str(), NULL);

	// Send "isready" command, so wait until answers "readyok".
	engine_send_command(engn, COMMAND_TYPE_GET_ANSW, IS_READY, answ);
}

void engine_parse_analisys_output(engine* engn, engine_line* line) {
	char word[MAX_WORD_SIZE] = { '\0' };
	int word_indx = 0, // Index of word string
		line_indx = 0; // Index of line
	char c_buff;
	bool is_word = false,  // Word flag
		 is_line = false,  // If word == multipv, than flag true
		 is_move = false;  // If word == pv, than flag true

	// Clean old info from engine, before reciving new info 
	clean_pipe_out(&engn->pipe_out_r);

	// Continue get data until receive "bestmove" word from engine
	while (memcmp(word, "bestmove", 9) != 0)
	{
		wait_for_answ(&engn->pipe_out_r);
		recieve_char(&engn->pipe_out_r, &c_buff);

		if (c_buff == ' ' || c_buff == '\n') { // null-characters zero word string
			if (is_word) {
				if (is_line) {
					line_indx = atoi(word); // Remember index of line
					is_line = false;
				}
				else if (is_move) { // Remember move from remembered line
					memcpy(line[line_indx - 1].move, word, sizeof(char) * MAX_MOVE_SIZE);
					is_move = false;
				}

				is_word = false;
				word_indx = 0;
				memset(word, '\0', sizeof(char) * MAX_WORD_SIZE);
			}
		}
		else
		{
			if (is_word == false) is_word = true;

			// Gather word
			word[word_indx++] = c_buff;

			if (memcmp(word, "multipv", 8) == 0) {
				is_line = true;
				is_word = false; // This is a little crutch to avoid logical error.
				word_indx = 0;
				memset(word, '\0', sizeof(char) * MAX_WORD_SIZE);
			}
			else if (memcmp(word, "pv", 3) == 0) {
				is_move = true;
				is_word = false; // The same as upper.
				word_indx = 0;
				memset(word, '\0', sizeof(char) * MAX_WORD_SIZE);
			}
		}
	}
}

engine_answers engine_identify_answer(char* answ) {
	if (answ[0] == '\0') {
		return NO_ANSW;
	}
	else if (strstr(answ, "uciok") != NULL)
	{
		return UCI_OK;
	}
	else if (strstr(answ, "readyok") != NULL) {
		return READY_OK;
	}
	else {
		return NORMAL_ANSW;
	}
}