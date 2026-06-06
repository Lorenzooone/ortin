#ifndef __ORTIN_ORTIN_ACTUAL_COMMANDS_HPP__
#define __ORTIN_ORTIN_ACTUAL_COMMANDS_HPP__

#include "tcharx.h"
#include "ISNitro.hpp"

int do_fullreset_cmd(ISNitro* connected_isne);
int do_reset_cmd(ISNitro* connected_isne);
int do_dump_isne_fw_out_cmd(ISNitro* connected_isne, std::string out_filepath);
int do_dump_ds_ipl_fw_out_cmd(ISNitro* connected_isne, std::string out_filepath);

#endif
