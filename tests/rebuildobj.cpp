/*
 * Auto-added header
 * File: tests/rebuild.cpp
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
#include <getopt.h>   // getopt_long
//------------------------------------------------------------------------------------------------------------------------------
#include "object_file.h"
#include "logger.h"
#include "fileops.h"
//------------------------------------------------------------------------------------------------------------------------------
/*
*	This file is a test program which purpose is to parse an object file (Elf32 format) and write it back as a copy,
* 	just for testing parsing/serialising functions of ObjectFile class
*/
//------------------------------------------------------------------------------------------------------------------------------
void print_help(const char* progname) {
    std::cout << "Usage: " << progname << " -i <input> -o <output>\n\n"
              << "Options:\n"
              << "  -i, --input   <file>   Input filename\n"
              << "  -o, --output  <file>   Output filename\n"
              << "  -h, --help             Show this help message\n";
}
//------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) {
    std::string input_file;
    std::string output_file;

    const char* short_opts = "i:o:h";
    const option long_opts[] = {
        {"input",  required_argument, nullptr, 'i'},
        {"output", required_argument, nullptr, 'o'},
        {"help",   no_argument,       nullptr, 'h'},
        {nullptr,  0,                 nullptr,  0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'i': input_file  = optarg; break;
            case 'o': output_file = optarg; break;
            case 'h': print_help(argv[0]); return 0;
            default:
                print_help(argv[0]);
                return -1;
        }
    }

    if (input_file.empty() || output_file.empty()) {
        LOG_ERROR("Both input (-i) and output (-o) files must be specified");
        print_help(argv[0]);
        return -1;
    }

    if (!std::filesystem::exists(input_file)) {
        LOG_ERROR("file not found: %s", input_file.c_str());
        return -1;
    }

    std::ifstream InputStream(input_file, std::ios::binary);
    std::vector<uint8_t> input_data(std::filesystem::file_size(input_file));
    InputStream.read(reinterpret_cast<char*>(input_data.data()), input_data.size());
    InputStream.close();

    std::vector<uint8_t> output_data;
    LOG_INFO("initial object size %d", (int)input_data.size());
    struct ar_hdr header; // placeholder
	ElfMan::ObjectFile obj(input_data.data(), input_data.size(), header, input_file);
	output_data = obj.serialize();
	LOG_INFO("resulting object size %d\n", (int)output_data.size());
    if (output_data.size() <= 0) {
        LOG_ERROR("serialising object file failed");
        return -1;
    }

    if (!Utils::FileOps::write_file(output_file, output_data)) {
        return -1;
    }

    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------
