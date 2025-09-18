/*
 * Auto-added header
 * File: static_library.h
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#ifndef STATIC_LIBRARY_H
#define STATIC_LIBRARY_H
//------------------------------------------------------------------------------------------------------------------------------
#include <vector>
#include <string>
#include <cstdint>
//------------------------------------------------------------------------------------------------------------------------------
#include "object_file.h"
//------------------------------------------------------------------------------------------------------------------------------
namespace ElfMan
{
//------------------------------------------------------------------------------------------------------------------------------
class StaticLibrary {
public:
    explicit StaticLibrary(const std::vector<uint8_t>& data);

    // Returns parsed object files
    const std::vector<std::shared_ptr<ArchiveObjectFile>>& getObjects() const { return objects; }
    std::vector<uint8_t> serialize();
    void reorder_symtab_and_relocations();
    // Debug dump of archive contents
    void dump();
    std::shared_ptr<ElfMan::Symbol> find_symbol(std::string sym_name);
    bool rename_symbol(std::string old_name, std::string new_name);
private:
    std::vector<std::shared_ptr<ArchiveObjectFile>> objects;
    std::string nameTable; // GNU string table for long filenames
    std::string symbolTable; // GNU string table for symbols
};
//------------------------------------------------------------------------------------------------------------------------------
} //namespace ElfMan
//------------------------------------------------------------------------------------------------------------------------------
#endif /*STATIC_LIBRARY_H*/