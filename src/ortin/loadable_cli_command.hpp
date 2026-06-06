#ifndef __ORTIN_ORTIN_LOADABLE_CLI_COMMAND_HPP__
#define __ORTIN_ORTIN_LOADABLE_CLI_COMMAND_HPP__

#include "tcharx.h"
#include "ISNitro.hpp"

// FIXME: gcc doesn't support printf attributes for wide strings.
#if defined(__GNUC__) && !defined(_WIN32)
# define ATTR_PRINTF(fmt, args) __attribute__ ((format (printf, (fmt), (args))))
#else
# define ATTR_PRINTF(fmt, args)
#endif

typedef int (*loadable_cli_command_function)(ISNitro* connected_isne, int argc, TCHAR *argv[]);

struct loadable_cli_command_t {
	std::string command;
	std::string command_syntax;
	std::string full_description;
	bool requires_isne_connected;
	int num_required_params;
	loadable_cli_command_function fn;
};

const loadable_cli_command_t* get_cli_command_t_for_id(int index);
int get_num_cli_command_t();
const loadable_cli_command_t* get_cli_command_t_for_command(int argc, TCHAR *argv[]);

void print_help(TCHAR* argv0);

/**
 * Print an error message.
 * @param argv0 Program name.
 * @param fmt Format string.
 * @param ... Arguments.
 */
void ATTR_PRINTF(2, 3) print_error(const TCHAR *argv0, const TCHAR *fmt, ...);

#endif
