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

// CLI commands
#include "loadable_cli_command.hpp"

// IS-NITRO
#include "ISNitro.hpp"

// Commands
#include "load-rom.hpp"
#include "avmode.hpp"
#include "actual_commands.hpp"

#include "tcharx.h"

#define CLI_CMD_BASE_POS 1

static int do_fullreset_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_reset_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_load_nds_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_no_reset_load_nds_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_enc_nds_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_set_av_mode_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_slot_on_off_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_slot1_emu_on_off_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_load_cart_slot1_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_dump_isne_fw_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_dump_ds_ipl_fw_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_print_unitinfo_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);
static int do_print_help_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]);

static const loadable_cli_command_t fullreset_cmd = {
	.command = "fullreset",
	.command_syntax = "fullreset",
	.full_description =	"  Do a full reset. This clears the first 32 KB of EMULATOR memory, disables\n"
						"  both slots, and resets the system.",
	.requires_isne_connected = true,
	.num_required_params = 0,
	.fn = do_fullreset_cli_cmd,
};

static const loadable_cli_command_t reset_cmd = {
	.command = "reset",
	.command_syntax = "reset",
	.full_description =	"  Do a soft reset. This resets the DS CPU only.",
	.requires_isne_connected = true,
	.num_required_params = 0,
	.fn = do_reset_cli_cmd,
};

static const loadable_cli_command_t load_nds_cmd = {
	.command = "load",
	.command_syntax = "load filename.nds",
	.full_description =	"  Load a Nintendo DS ROM image. If the image has a decrypted secure area,\n"
						"  it will be re-encrypted on load.",
	.requires_isne_connected = true,
	.num_required_params = 1,
	.fn = do_load_nds_cli_cmd,
};

static const loadable_cli_command_t noreset_load_nds_cmd = {
	.command = "noreset_load",
	.command_syntax = "noreset_load filename.nds",
	.full_description =	"  Load a Nintendo DS ROM image. If the image has a decrypted secure area,\n"
						"  it will be re-encrypted on load. The device is not reset.",
	.requires_isne_connected = true,
	.num_required_params = 1,
	.fn = do_no_reset_load_nds_cli_cmd,
};

static const loadable_cli_command_t enc_nds_cmd = {
	.command = "enc_nds",
	.command_syntax = "enc_nds filename.nds out_filename.nds",
	.full_description =	"  Encrypts the secure area of a Nintendo DS ROM image.\n"
						"  Useful for certain programs.",
	.requires_isne_connected = false,
	.num_required_params = 2,
	.fn = do_enc_nds_cli_cmd,
};

static const loadable_cli_command_t avmode_cmd = {
	.command = "avmode",
	.command_syntax = "avmode av1 av2 [--bgcolor=COLOR] [--deflicker=DEFLICKER]",
	.full_description =	"  Set the AV mode settings. av1/av2 can be one of the following\n"
						"  primary mode characters:\n"
						"    N: No image. Disables the output entirely.\n"
						"    U: Upper screen image.\n"
						"    L: Lower screen image.\n"
						"    B: Both screen images, stacked on top of each other.\n"
						"  The following additional characters can be provided as modifiers:\n"
						"    I: Use interlaced output.\n"
						"    A: Do not use the correct aspect ratio.\n"
						"  Extra options:\n"
						"    -b, --bgcolor=COLOR       Specify a custom background color. (24-bit hex)\n"
						"                              Example: FF8000 - default is black (000000)\n"
						"    -d, --deflicker=DEFLICKER Deflicker mode: none, normal, alternate.\n"
						"                              Default is none.",
	.requires_isne_connected = true,
	.num_required_params = 2,
	.fn = do_set_av_mode_cli_cmd,
};

static const loadable_cli_command_t dump_isne_fw_cmd = {
	.command = "dump_isne_fw",
	.command_syntax = "dump_isne_fw [out_filename]",
	.full_description =	"  Dumps the firmware of the IS Nitro Emulator to \"out_filename\".\n"
						"  Default is fw_isne_dump_\'#SERIAL\'.bin",
	.requires_isne_connected = true,
	.num_required_params = 0,
	.fn = do_dump_isne_fw_cmd,
};

static const loadable_cli_command_t dump_ds_ipl_fw_cmd = {
	.command = "dump_ds_ipl_fw",
	.command_syntax = "dump_ds_ipl_fw [out_filepath_preamble]",
	.full_description =	"  Dumps the firmware of the DS IPL, BIOS7 and BIOS9\n"
						"  to \"out_filepath_preamble\"fw_ds_ipl.bin,\n"
						"  \"out_filepath_preamble\"bios9.bin and \"out_filepath_preamble\"bios7.bin\n"
						"  Default is dump_\'#SERIAL\'_\n"
						"  To be used together with dsbf_dump, by selecting \"To Emulated GBA ROM\".\n"
						"  The .nds file should be located in either the current directory, or the\n"
						"  directory of the program.\n"
						"  The dsbf_dump used by default is the one also available at:\n"
						"  https://github.com/Lorenzooone/dsbf_dump/releases/tag/1.0.isne",
	.requires_isne_connected = true,
	.num_required_params = 0,
	.fn = do_dump_ds_ipl_fw_cmd,
};

static const loadable_cli_command_t slot1_emu_on_off_cmd = {
	.command = "slot1emu",
	.command_syntax = "slot1emu on/off",
	.full_description =	"  Turns on or off slot 1 (DS) emulation.",
	.requires_isne_connected = true,
	.num_required_params = 1,
	.fn = do_slot1_emu_on_off_cli_cmd,
};

static const loadable_cli_command_t slot_on_off_cmd = {
	.command = "slot",
	.command_syntax = "slot on/off 1/2",
	.full_description =	"  Turns on or off slot 1 (DS) or slot 2 (GBA).",
	.requires_isne_connected = true,
	.num_required_params = 2,
	.fn = do_slot_on_off_cli_cmd,
};

static const loadable_cli_command_t load_cartridge_slot_1_cmd = {
	.command = "launchcartslot1",
	.command_syntax = "launchcartslot1 [slot1launch_path]",
	.full_description =	"  Lauches the cartridge present in slot 1 (DS) by using slot1launch.\n"
						"  Default slot1launch_path is \"./slot1launch.dsi\".\n"
						"  A different one may be specified if needed.\n"
						"  The slot1launch used by default is the one also available at:\n"
						"  https://github.com/Lorenzooone/Simple-DS-Slot-1-Launcher/releases",
	.requires_isne_connected = true,
	.num_required_params = 0,
	.fn = do_load_cart_slot1_cli_cmd,
};

static const loadable_cli_command_t print_basic_info_cmd = {
	.command = "unitinfo",
	.command_syntax = "unitinfo",
	.full_description =	"  Display basic information about the connected unit.",
	.requires_isne_connected = true,
	.num_required_params = 0,
	.fn = do_print_unitinfo_cmd,
};

static const loadable_cli_command_t help_cmd = {
	.command = "help",
	.command_syntax = "help",
	.full_description =	"  Display this help and exit.",
	.requires_isne_connected = false,
	.num_required_params = 0,
	.fn = do_print_help_cmd,
};

static const loadable_cli_command_t* all_cli_cmds[] = {
	&fullreset_cmd,
	&reset_cmd,
	&load_nds_cmd,
	&noreset_load_nds_cmd,
	&enc_nds_cmd,
	&avmode_cmd,
	&dump_isne_fw_cmd,
	&dump_ds_ipl_fw_cmd,
	&slot_on_off_cmd,
	&slot1_emu_on_off_cmd,
	&load_cartridge_slot_1_cmd,
	&print_basic_info_cmd,
	&help_cmd,
};

int get_num_cli_command_t() {
	return sizeof(all_cli_cmds) / sizeof(all_cli_cmds[0]);
}

const loadable_cli_command_t* get_cli_command_t_for_id(int index) {
	if((index < 0) || (index >= get_num_cli_command_t()))
		index = 0;
	return all_cli_cmds[index];
}

const loadable_cli_command_t* get_cli_command_t_for_command(int argc, TCHAR *argv[]) {
	const loadable_cli_command_t* found_cli_cmd = NULL;

	if(argc <= CLI_CMD_BASE_POS) {
		print_help(argv[0]);
		return found_cli_cmd;
	}

	std::string cli_cmd_str = std::string(argv[CLI_CMD_BASE_POS]);

	for(int i = 0; i < get_num_cli_command_t(); i++) {
		const loadable_cli_command_t* curr_cli_cmd = get_cli_command_t_for_id(i);
		if(curr_cli_cmd->command == cli_cmd_str) {
			found_cli_cmd = curr_cli_cmd;
			break;
		}
	}

	if(found_cli_cmd == NULL) {
		print_error(argv[0], _T("unrecognized command '%s'"), argv[CLI_CMD_BASE_POS]);
		return found_cli_cmd;
	}

	if(argc < (found_cli_cmd->num_required_params + 1 + CLI_CMD_BASE_POS)) {
		print_error(argv[0], _T("not enough parameters for command '%s'"), argv[CLI_CMD_BASE_POS]);
		return NULL;
	}

	return found_cli_cmd;
}

// Separate parsing and logic...

static std::string tolower_str(std::string str) {
	// Converting the std::string to lower case
	for_each(str.begin(), str.end(), [](char& c) {
		c = _totlower(c);
	});
	return str;
}

static bool is_file_accessible (const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

static std::string get_dir_of(const std::string& fname) {
	size_t pos = fname.find_last_of("\\/");
	return (std::string::npos == pos)
		? ""
		: fname.substr(0, pos);
}

static std::string get_accessible_path_of(const std::string& base_filepath, const TCHAR *argv0) {
	std::string file_filepath = base_filepath;

	if(!is_file_accessible(file_filepath)) {
		file_filepath = get_dir_of(std::string(argv0)) + "/" + base_filepath;
		if(!is_file_accessible(file_filepath))
			return "";
	}
	return file_filepath;
}

void ATTR_PRINTF(2, 3) print_error(const TCHAR *argv0, const TCHAR *fmt, ...)
{
	if (fmt != NULL) {
		va_list ap;
		va_start(ap, fmt);
		_ftprintf(stderr, _T("%s: "), argv0);
		_vftprintf(stderr, fmt, ap);
		va_end(ap);

		fputc('\n', stderr);
	}

	_ftprintf(stderr, _T("Try `%s` help` for more information.\n"), argv0);
}

static int do_fullreset_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	return do_fullreset_cmd(connected_isne);
}

static int do_reset_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	return do_reset_cmd(connected_isne);
}

static int do_load_nds_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	return load_nds_rom(connected_isne, argv[CLI_CMD_BASE_POS + 1]);
}

static int do_no_reset_load_nds_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	return load_nds_rom(connected_isne, argv[CLI_CMD_BASE_POS + 1], false);
}

static int do_enc_nds_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	return enc_nds_rom(argv[CLI_CMD_BASE_POS + 1], argv[CLI_CMD_BASE_POS + 2]);
}

static int do_set_av_mode_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	// avmode options.
	uint32_t bg_color = 0;
	NitroAVDeflicker_e deflicker = NITRO_AV_DEFLICKER_DISABLED;
	// TODO: Allow customization once we figure out how to get
	// rotation set up properly.
	NitroAVRotation_e rotation = NITRO_AV_ROTATION_NONE;

	while (true) {
		static const struct option long_options[] = {
			{_T("bgcolor"),		required_argument,	0, _T('b')},
			{_T("deflicker"),	required_argument,	0, _T('d')},

			{NULL, 0, 0, 0}
		};

		int c = getopt_long(argc, argv, _T("b:d:"), long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
			case _T('b'): {
				// Background color.
				if (!optarg || optarg[0] == '\0') {
					// NULL?
					print_error(argv[0], _T("no background color specified"));
					return EXIT_FAILURE;
				}

				char *endptr = nullptr;
				bg_color = strtoul(optarg, &endptr, 16);
				if (*endptr != '\0') {
					print_error(argv[0], _T("background color is invalid (should be 24-bit hex)"));
					return EXIT_FAILURE;
				}
				break;
			}

			case _T('d'):
				// Deflicker.
				if (!optarg || optarg[0] == '\0') {
					// NULL?
					print_error(argv[0], _T("no deflicker mode specified"));
					return EXIT_FAILURE;
				}

				if (_tcsicmp(optarg, _T("none"))) {
					deflicker = NITRO_AV_DEFLICKER_DISABLED;
				} else if (_tcsicmp(optarg, _T("normal"))) {
					deflicker = NITRO_AV_DEFLICKER_NORMAL;
				} else if (_tcsicmp(optarg, _T("alternate")) ||
					   _tcsicmp(optarg, _T("alt")))
				{
					deflicker = NITRO_AV_DEFLICKER_ALTERNATE;
				} else {
					print_error(argv[0], _T("deflicker mode is invalid"));
					return EXIT_FAILURE;
				}
				break;

			case _T('?'):
			default:
				print_error(argv[0], NULL);
				return EXIT_FAILURE;
		}
	}

	if (argc < optind + 3) {
		print_error(argv[0], _T("AV mode parameters not specified"));
		return EXIT_FAILURE;
	} 

	return set_av_mode(connected_isne, argv[optind + 1], argv[optind + 2], bg_color, deflicker, rotation);
}

static int do_slot_on_off_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	bool command_on = false;
	int _slot = strtol(argv[CLI_CMD_BASE_POS + 2], nullptr, 10);

	std::string command_str = tolower_str(std::string(argv[CLI_CMD_BASE_POS + 1]));
	if(command_str != "on" && command_str != "off") {
		print_error(argv[0], _T("Command '%s' is not valid"), argv[CLI_CMD_BASE_POS + 1]);
		return EXIT_FAILURE;
	}
	command_on = command_str == "on";

	if(_slot != 1 && _slot != 2) {
		print_error(argv[0], _T("Slot number '%s' is not valid"), argv[CLI_CMD_BASE_POS + 2]);
		return EXIT_FAILURE;
	}

	return connected_isne->setSlotPower(_slot, command_on);
}

static int do_slot1_emu_on_off_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	bool command_on = false;

	std::string command_str = tolower_str(std::string(argv[CLI_CMD_BASE_POS + 1]));
	if(command_str != "on" && command_str != "off") {
		print_error(argv[0], _T("Command '%s' is not valid"), argv[CLI_CMD_BASE_POS + 1]);
		return EXIT_FAILURE;
	}
	command_on = command_str == "on";

	return connected_isne->changeSlot1Emulation(command_on);
}

static int do_load_cart_slot1_cli_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	const std::string base_slot1launch_filepath = "slot1launch.dsi";
	const std::string base_slot1launch_filepath_alt = "slot1launch.nds";
	std::string slot1launch_filepath = "";

	if(argc >= (CLI_CMD_BASE_POS + 2)) {
		slot1launch_filepath = std::string(argv[CLI_CMD_BASE_POS + 1]);
		if(!is_file_accessible(slot1launch_filepath)) {
			print_error(argv[0], _T("could not find %s"), slot1launch_filepath.c_str());
			return EXIT_FAILURE;
		}
	}
	else {
		slot1launch_filepath = get_accessible_path_of(base_slot1launch_filepath, argv[0]);
		std::string slot1launch_filepath_alt = get_accessible_path_of(base_slot1launch_filepath_alt, argv[0]);
		if(slot1launch_filepath == "") {
			if(slot1launch_filepath_alt == "") {
				print_error(argv[0], _T("could not find %s or %s"), base_slot1launch_filepath.c_str(), base_slot1launch_filepath_alt.c_str());
				return EXIT_FAILURE;
			}
			slot1launch_filepath = slot1launch_filepath_alt;
		}
	}

	return do_launch_cart_slot1_cmd(connected_isne, slot1launch_filepath);
}

static int do_dump_isne_fw_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	std::string out_filepath;

	if(argc >= (CLI_CMD_BASE_POS + 2))
		out_filepath = std::string(argv[CLI_CMD_BASE_POS + 1]);
	else {
		std::string serial_str = "";
		int ret = connected_isne->getSerial(&serial_str);
		if(ret < 0)
			return ret;
		out_filepath = "fw_isne_dump_" + serial_str + ".bin";
	}

	return do_dump_isne_fw_out_cmd(connected_isne, out_filepath);
}

static int do_dump_ds_ipl_fw_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	std::string out_filepath_preamble;
	const std::string base_dsbf_dump_filepath = "dsbf_dump.nds";
	std::string dsbf_dump_filepath = get_accessible_path_of(base_dsbf_dump_filepath, argv[0]);

	if(dsbf_dump_filepath == "") {
		print_error(argv[0], _T("could not find %s"), base_dsbf_dump_filepath.c_str());
		return EXIT_FAILURE;
	}

	if(argc >= (CLI_CMD_BASE_POS + 2))
		out_filepath_preamble = std::string(argv[CLI_CMD_BASE_POS + 1]);
	else {
		std::string serial_str = "";
		int ret = connected_isne->getSerial(&serial_str);
		if(ret < 0)
			return ret;
		out_filepath_preamble = "dump_" + serial_str + "_";
	}

	return do_dump_ds_ipl_fw_out_cmd(connected_isne, out_filepath_preamble, dsbf_dump_filepath);
}

static int do_print_unitinfo_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	std::string serial = "";
	uint32_t ram_size = 0;
	uint16_t fw_info = 0;

	int ret = connected_isne->getSerial(&serial);
	if (ret < 0)
		return ret;
	fprintf(stdout, "Serial: %s\n", serial.c_str());
	ret = connected_isne->getDeviceInstalledRAM(&ram_size);
	if (ret < 0)
		return ret;
	fprintf(stdout, "Installed RAM: %u MB\n", ram_size / (1024 * 1024));
	ret = connected_isne->getDeviceCapabilities(&fw_info);
	if (ret < 0)
		return ret;
	fprintf(stdout, "Capabilities: %04X\n", fw_info);

	return ret;
}

static int do_print_help_cmd(ISNitro* connected_isne, int argc, TCHAR *argv[]) {
	print_help(argv[0]);
	return 0;
}

void print_help(TCHAR* argv0) {
	fputs("This program is licensed under the GNU GPL v2.\n"
		"For more information, visit: http://www.gnu.org/licenses/\n"
		"\n", stdout);

	fputs("Syntax: ", stdout);
	_fputts(argv0, stdout);
	fputs("\n"
		"Supported commands:\n", stdout);

	for(int i = 0; i < get_num_cli_command_t(); i++) {
		const loadable_cli_command_t* curr_cli_cmd = get_cli_command_t_for_id(i);
		fputs("\n", stdout);
		fputs(curr_cli_cmd->command_syntax.c_str(), stdout);
		fputs("\n", stdout);
		fputs(curr_cli_cmd->full_description.c_str(), stdout);
		fputs("\n", stdout);
	}
}
