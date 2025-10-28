/*
 * Auto-added header
 * File: tests/insert.cpp
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
#include "static_library.h"
#include "symbol.h"
#include "logger.h"
#include "fileops.h"
//------------------------------------------------------------------------------------------------------------------------------
/*
*	This file is a test program which purpose is to parse an object file (Elf32 format) and insert a specific symbol
* 	into it
*/
//------------------------------------------------------------------------------------------------------------------------------
void print_help(const char* progname) {
    std::cout << "Usage: " << progname << " -i <file> [ -o <file> ] -s <symbol>\n\n"
              << "Options:\n"
              << "  -i, --input   <file>   Input filename\n"
              << "  -o, --output  <file>   Output filename\n"
              << "  -s, --symbol  <symbol> Symbol name\n"
              << "  -h, --help             Show this help message\n";
}
//------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) {
    std::string input_file;
    std::string output_file;
    std::string symbol_name;
    std::string object_name;

    const char* short_opts = "i:o:s:h";
    const option long_opts[] = {
        {"input",  required_argument, nullptr, 'i'},
        {"output", required_argument, nullptr, 'o'},
        {"symbol", required_argument, nullptr, 's'},
        {"help",   no_argument,       nullptr, 'h'},
        {nullptr,  0,                 nullptr,  0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'i': input_file  = optarg; break;
            case 'o': output_file = optarg; break;
            case 's': symbol_name = optarg; break;
            case 'h': print_help(argv[0]); return 0;
            default:
                print_help(argv[0]);
                return -1;
        }
    }

    if (input_file.empty() || symbol_name.empty()) {
        LOG_ERROR("Input file (-i) and symbol name (-s) must be specified");
        print_help(argv[0]);
        return -1;
    }

    if (output_file.empty())
        output_file = input_file;

    if (!std::filesystem::exists(input_file)) {
        LOG_ERROR("file not found: %s", input_file.c_str());
        return -1;
    }

    std::ifstream InputStream(input_file, std::ios::binary);
    std::vector<uint8_t> input_data(std::filesystem::file_size(input_file));
    InputStream.read(reinterpret_cast<char*>(input_data.data()), input_data.size());
    InputStream.close();
    struct ar_hdr header;
    ElfMan::ObjectFile elfobj(input_data.data(), input_data.size(), header, input_file);
    LOG_INFO("inserting symbol %s", symbol_name.c_str());
    std::shared_ptr<ElfMan::Symbol> wr_sym = elfobj.insert_undefined_global_function(symbol_name, true);

    std::vector<uint8_t> output_data = elfobj.serialize();
    if (Utils::FileOps::write_file(output_file, output_data)) {
        return -1;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------
