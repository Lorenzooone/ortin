// C includes.
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>	// FIXME: usleep() for Windows

// C++ includes. (C namespace)
#include <cstdio>
#include <cstring>

#include <fstream>

#include "actual_commands.hpp"

#define BUFFER_SIZE_FULLRESET 0x8000

#define BUFFER_SIZE_DUMP_ISNE_FW 0xE0000

#define BUFFER_SIZE_DUMP_DS_IPL_FW 0x40000

int do_fullreset_cmd(ISNitro* connected_isne) {
	// Full Reset: Wipe the first 32 KB of EMULATOR memory and reset the system.
	uint8_t *zerobytes = new uint8_t[BUFFER_SIZE_FULLRESET];
	int ret = connected_isne->writeEmulationMemory(1, 0, zerobytes, BUFFER_SIZE_FULLRESET);
	delete zerobytes;
	if(ret < 0)
		return ret;
	return connected_isne->fullReset();
}

int do_reset_cmd(ISNitro* connected_isne) {
	// Reset: Reset the DS CPU only.
	int ret = connected_isne->ndsReset(true);
	if(ret < 0)
		return ret;
	usleep(500000);
	return connected_isne->ndsReset(false);
}

int do_dump_isne_fw_out_cmd(ISNitro* connected_isne, std::string out_filepath) {
	uint8_t* buffer = new uint8_t[BUFFER_SIZE_DUMP_ISNE_FW];
	int ret = connected_isne->readNECMemory(0x210000, buffer, BUFFER_SIZE_DUMP_ISNE_FW);
	if(ret)
		fprintf(stderr, "Read failure");
	else {
		std::ofstream fs(out_filepath, std::ios::out | std::ios::binary);
		fs.write((const char*)buffer, BUFFER_SIZE_DUMP_ISNE_FW);
		fs.close();
	}
	delete buffer;
	return ret;
}

int do_dump_ds_ipl_fw_out_cmd(ISNitro* connected_isne, std::string out_filepath) {
	uint8_t* buffer = new uint8_t[BUFFER_SIZE_DUMP_DS_IPL_FW];
	int ret = connected_isne->readNECMemory(0x0F800000, buffer, BUFFER_SIZE_DUMP_DS_IPL_FW);
	if(ret) {
		fprintf(stderr, "Read failure");
		delete buffer;
		return ret;
	}
	else {
		std::ofstream fs(out_filepath, std::ios::out | std::ios::binary);
	    fs.write((const char*)buffer, BUFFER_SIZE_DUMP_DS_IPL_FW);
	    fs.close();
	}

	buffer[0] = 0x7E;
	buffer[1] = 0;
	ret = connected_isne->writeNECMemory(0x0F841000, buffer, 2);
	delete buffer;

	if(ret)
		fprintf(stderr, "Write failure");
	return ret;
}
