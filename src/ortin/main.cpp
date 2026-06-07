/***************************************************************************
 * Ortin (IS-NITRO management) (ortin CLI)                                 *
 * main.cpp: Command line interface.                                       *
 *                                                                         *
 * Copyright (c) 2020 by David Korth.                                      *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "config.version.h"
#include "git.h"

// C includes.
#include <getopt.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>	// FIXME: usleep() for Windows

// C++ includes. (C namespace)
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <fstream>

// C++ includes.
#include <algorithm>
#include <locale>

// libusb
#include <libusb.h>

// IS-NITRO
#include "ISNitro.hpp"

// Commands
#include "loadable_cli_command.hpp"

#include "tcharx.h"
#ifdef _MSC_VER
# define ORTIN_CDECL __cdecl
#else
# define ORTIN_CDECL
#endif

int ORTIN_CDECL _tmain(int argc, TCHAR *argv[])
{
	// Set the C and C++ locales.
	std::locale::global(std::locale(""));

	puts("Ortin Tool v" VERSION_STRING "\n"
		"Copyright (c) 2020 by David Korth.\n"
		"This program is NOT licensed or endorsed by Nintendo Co, Ltd."
	);
#ifdef RP_GIT_VERSION
	puts(RP_GIT_VERSION);
# ifdef RP_GIT_DESCRIBE
	puts(RP_GIT_DESCRIBE);
# endif
#endif
	putchar('\n');

	const loadable_cli_command_t* found_cli_cmd = get_cli_command_t_for_command(argc, argv);

	if(found_cli_cmd == NULL)
		return EXIT_FAILURE;

	if(!found_cli_cmd->requires_isne_connected)
		return found_cli_cmd->fn(NULL, argc, argv);

	int status = libusb_init(nullptr);
	if (status < 0) {
		fprintf(stderr, "*** ERROR: libusb_init() failed: %s\n", libusb_error_name(status));
		return EXIT_FAILURE;
	}

	ISNitro *nitro = new ISNitro();
	if ((!nitro) || (!nitro->isOpen())) {
		fprintf(stderr, "*** ERROR: Unable to open the IS-NITRO unit.\n");
		libusb_exit(nullptr);
		return EXIT_FAILURE;
	}

	int ret = found_cli_cmd->fn(nitro, argc, argv);

	delete nitro;
	libusb_exit(nullptr);
	return ret;
}
