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
#include "static_library.h"
#include "logger.h"
#include "fileops.h"
//------------------------------------------------------------------------------------------------------------------------------
/*
*	This file is a test program which purpose is to parse an static library ar file (Elf32 format) and write it back as a copy,
* 	but with specified symbol visibility changed to GLOBAL
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
        LOG_ERROR("Both input file (-i) and symbol name (-s) must be specified");
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

    LOG_INFO("initial archive size %d", (int)input_data.size());
    std::vector<uint8_t> output_data;

    ElfMan::StaticLibrary staticlib(input_data);
    staticlib.dump();

    std::shared_ptr<ElfMan::Symbol> symbol = staticlib.find_symbol(symbol_name);
    if (!symbol) {
        LOG_ERROR("symbol not found: %s", symbol_name.c_str());
        return 0;
    }
    LOG_INFO("symbol found: %s, object %s, offset %08X", symbol_name.c_str(), symbol->object->filename().c_str(), symbol->offset());
    if (STB_GLOBAL == symbol->bind())
        LOG_INFO("symbol already global - skip");
    else {
        LOG_INFO("making symbol global: %s", symbol->name().c_str());
        symbol->set_global();
        staticlib.reorder_symtab_and_relocations();
    }
    output_data = staticlib.serialize();
    LOG_INFO("resulting archive size %d\n", (int)output_data.size());
    if (output_data.size() <= 0) {
        LOG_ERROR("serializing archive file failed");
        return -1;
    }

    if (!Utils::FileOps::write_file(output_file, output_data)) {
        return -1;
    }

    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------
