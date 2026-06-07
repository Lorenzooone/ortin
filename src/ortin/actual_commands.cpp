// C includes.
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>	// FIXME: usleep() for Windows

// C++ includes. (C namespace)
#include <cstdio>
#include <cstring>

#include <fstream>

#include "actual_commands.hpp"
#include "load-rom.hpp"

#define BUFFER_SIZE_FULLRESET 0x8000

#define BUFFER_SIZE_DUMP_ISNE_FW 0x400000

#define ASDRAM_SIZE 0x40000

static void write_le32(uint8_t* data, uint32_t value) {
	data[0] = value & 0xFF;
	data[1] = (value >> 8) & 0xFF;
	data[2] = (value >> 16) & 0xFF;
	data[3] = (value >> 24) & 0xFF;
}

static uint32_t read_le32(uint8_t* data) {
	uint32_t out = 0;
	out |= data[0];
	out |= data[1] << 8;
	out |= data[2] << 16;
	out |= data[3] << 24;
	return out;
}

static uint32_t calc_checksum_of_buffer(uint8_t* data, size_t size) {
	uint32_t out = 0;
	for(size_t i = 0; i < ((size + 3) >> 2); i++)
		out += read_le32(data + (i * 4));
	return out;
}

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

	int ret = connected_isne->readNECMemory(0, buffer, BUFFER_SIZE_DUMP_ISNE_FW);
	if(ret) {
		fprintf(stderr, "Read failure");
		delete buffer;
		return ret;
	}

	std::ofstream fs(out_filepath, std::ios::out | std::ios::binary);
	fs.write((const char*)buffer, BUFFER_SIZE_DUMP_ISNE_FW);
	fs.close();

	delete buffer;
	return ret;
}

static int _do_dump_ds_ipl_fw_out_cmd(ISNitro* connected_isne, uint8_t* buffer, uint8_t** buffer_save, std::string out_filepath_preamble, std::string dsbf_dump_filepath) {
	uint8_t base_data[0x10];
	memset(base_data, 0, sizeof(base_data));
	int ret = connected_isne->updateDebugButtonState(true);
	if(ret < 0) {
		fprintf(stderr, "Debug button set failure");
		return ret;
	}

	// Need to write like this...
	ret = connected_isne->writeEmulationMemory(2, 0, base_data, sizeof(base_data));
	if(ret) {
		fprintf(stderr, "Write failure");
		return ret;
	}

	ret = load_nds_rom(connected_isne, dsbf_dump_filepath.c_str());
	if(ret < 0)
		return ret;

	const int num_dumps = 3;
	std::string attached_paths[num_dumps] = {"fw_ds_ipl.bin", "bios7.bin", "bios9.bin"};
	std::string corresponding_string[num_dumps] = {"Firmware", "BIOS NDS7", "BIOS NDS9"};

	for(int i = 0; i < num_dumps; i++) {

		const uint32_t word_known = 0xDEADBEEF;
		while(read_le32(base_data) != word_known) {
			ret = connected_isne->readNECMemory(0x0F800000, base_data, sizeof(base_data));
			if(ret) {
				fprintf(stderr, "Read failure");
				return ret;
			}
		}

		write_le32(base_data + 4, ~word_known);
		ret = connected_isne->writeEmulationMemory(2, 0, base_data, sizeof(base_data));
		if(ret) {
			fprintf(stderr, "Write failure");
			return ret;
		}
		ret = connected_isne->updateDebugButtonState(false);
		if(ret) {
			fprintf(stderr, "Debug button set failure");
			return ret;
		}

		// These will be updated...
		size_t num_transfers = 1;
		size_t full_size = 0;
		size_t desc_size = 0;

		std::string description = "";
		if((*buffer_save) != NULL) {
			delete *buffer_save;
			*buffer_save = NULL;
		}

		// Do the portions...
		for(size_t j = 0; j < num_transfers; j++) {

			// Wait for init write
			while(read_le32(base_data + 4) != 0) {
				ret = connected_isne->readNECMemory(0x0F800000, base_data, sizeof(base_data));
				if(ret) {
					fprintf(stderr, "Read failure");
					return ret;
				}
			}

			// Wait for done write
			while(read_le32(base_data) != word_known) {
				ret = connected_isne->readNECMemory(0x0F800000, base_data, sizeof(base_data));
				if(ret) {
					fprintf(stderr, "Read failure");
					return ret;
				}
			}

			// Actually read buffer
			ret = connected_isne->readNECMemory(0x0F800000, buffer, ASDRAM_SIZE);
			if(ret) {
				fprintf(stderr, "Main read failure");
				return ret;
			}

			if(j == 0) {
				full_size = read_le32(buffer + 8);
				desc_size = read_le32(buffer + 0xC);
				description = std::string((char*)buffer + 0x10);
				*buffer_save = new uint8_t[full_size];
			}
			uint8_t* data_post_desc = buffer + 0x10 + desc_size;
			num_transfers = read_le32(data_post_desc);
			size_t pos_save_buffer = read_le32(data_post_desc + 4);
			size_t size_save_buffer = read_le32(data_post_desc + 8);
			uint32_t expected_checksum_slot = read_le32(data_post_desc + 0xC);
			uint32_t got_checksum_slot = calc_checksum_of_buffer(data_post_desc + 0x10, size_save_buffer);

			if(expected_checksum_slot != got_checksum_slot) {
				fprintf(stderr, "Checksum failure");
				return EXIT_FAILURE;
			}

			memcpy((*buffer_save) + pos_save_buffer, data_post_desc + 0x10, size_save_buffer);

			ret = connected_isne->readNECMemory(0x0F800000, base_data, sizeof(base_data));
			if(ret) {
				fprintf(stderr, "Read failure");
				return ret;
			}
			// Signal done reading
			write_le32(base_data + 4, ~word_known);
			ret = connected_isne->writeEmulationMemory(2, 0, base_data, sizeof(base_data));
			if(ret) {
				fprintf(stderr, "Write failure");
				return ret;
			}
		}

		ret = connected_isne->updateDebugButtonState(true);
		if(ret) {
			fprintf(stderr, "Debug button set failure");
			return ret;
		}

		while(read_le32(base_data) != (word_known ^ 0x11111111)) {
			ret = connected_isne->readNECMemory(0x0F800000, base_data, sizeof(base_data));
			if(ret) {
				fprintf(stderr, "Read failure");
				return ret;
			}
		}

		int index_in_strs = -1;
		for(int i = 0; i < num_dumps; i++)
			if(corresponding_string[i] == description) {
				index_in_strs = i;
				break;
			}

		if(index_in_strs != -1) {
			std::ofstream fs(out_filepath_preamble + attached_paths[index_in_strs], std::ios::out | std::ios::binary);
			fs.write((const char*)*buffer_save, full_size);
			fs.close();
		}
		else
			fprintf(stderr, "WARNING: Unknown transfer %s", description.c_str());
	}

	ret = connected_isne->updateDebugButtonState(false);
	if(ret) {
		fprintf(stderr, "Debug button set failure");
		return ret;
	}

	return ret;
}

int do_dump_ds_ipl_fw_out_cmd(ISNitro* connected_isne, std::string out_filepath, std::string dsbf_dump_filepath) {
	uint8_t* buffer = new uint8_t[ASDRAM_SIZE];
	uint8_t* buffer_save = NULL;

	int ret = _do_dump_ds_ipl_fw_out_cmd(connected_isne, buffer, &buffer_save, out_filepath, dsbf_dump_filepath);

	if(buffer_save != NULL)
		delete buffer_save;
	delete buffer;
	return ret;
}
