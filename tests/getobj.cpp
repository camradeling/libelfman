/*
 * Auto-added header
 * File: tests/getobj.cpp
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <ar.h>
#include <getopt.h>
//------------------------------------------------------------------------------------------------------------------------------
#include "static_library.h"
#include "object_file.h"
#include "logger.h"
#include "fileops.h"
//------------------------------------------------------------------------------------------------------------------------------
/*
*	This file is a test program which purpose is to parse a static library (Elf32 format) and extract a particular object file from it.
*	Basically a clone of "ar x" call functionality, just for testing
*/
//------------------------------------------------------------------------------------------------------------------------------
void print_usage(const char* progname) {
    std::cout << "Usage: " << progname << " -i <input_file> [-o <output_file>]\n"
              << "Options:\n"
              << "  -i, --input   	Input file name (required)\n"
              << "  -f, --filename  Output file name (optional, default: <input_file>_mod)\n"
              << "  -h, --help    	Show this help\n";
}
//------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    std::string input_file;
    std::string output_file;

    const struct option long_opts[] = {
        {"input",  		required_argument, nullptr, 'i'},
        {"filename", 	required_argument, nullptr, 'f'},
        {"help",   		no_argument,       nullptr, 'h'},
        {nullptr,  0,                 nullptr,  0 }
    };

    int opt;
    int opt_index = 0;
    while ((opt = getopt_long(argc, argv, "i:f:h", long_opts, &opt_index)) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'f':
                output_file = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return -1;
        }
    }

    if (input_file.empty()) {
        LOG_ERROR("input file not specified\n");
        print_usage(argv[0]);
        return -1;
    }

    if (!std::filesystem::exists(input_file)) {
        LOG_ERROR("file not found: %s\n", input_file.c_str());
        return -1;
    }

    std::ifstream InputStream(input_file, std::ios::binary);
    if (!InputStream.is_open()) {
        LOG_ERROR("cannot open file: %s\n", input_file.c_str());
        return -1;
    }

    std::vector<uint8_t> StaticLib(std::filesystem::file_size(input_file));
    InputStream.read((char*)StaticLib.data(), StaticLib.size());
    InputStream.close();
    std::vector<uint8_t> output_data;
    ElfMan::StaticLibrary staticlib(StaticLib);
    staticlib.dump();
    for (auto &object : staticlib.getObjects()) {
    	if (object->filename() == output_file) {
    		output_data = object->serialize();
    		break;
    	}
    }

    if (output_data.size() <= 0) {
        LOG_ERROR("file object %s not found\n", output_file.c_str());
        return -1;
    }

    if (Utils::FileOps::write_file(output_file, output_data)) {
        return -1;
    }

    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------
