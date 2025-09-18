/*
 * Auto-added header
 * File: static_library.cpp
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <elf.h>
#include <ar.h>
//------------------------------------------------------------------------------------------------------------------------------
#include "static_library.h"
#include "object_file.h"
#include "convenient.h"
#include "logger.h"
//------------------------------------------------------------------------------------------------------------------------------
// Parse AR archive
ElfMan::StaticLibrary::StaticLibrary(const std::vector<uint8_t>& data)
{
    // Check global archive magic + header length
    if (data.size() < SARMAG + sizeof(struct ar_hdr))
        throw std::runtime_error("Not a valid ar archive");

    ElfMan::Memory::InputMemoryStream library_stream(data.data(), data.size());
    // read Magic bytes
    char magic[SARMAG] = {0};
    library_stream.read(magic);
    // Check global archive magic value
    if (memcmp(magic, ARMAG, SARMAG))
        throw std::runtime_error("Not a valid ar archive");

    std::vector<uint8_t> object;
    struct ar_hdr header;
    while (library_stream) {
        library_stream.read(header);
        if (Utils::Convenient::trim(std::string(header.ar_fmag, sizeof(header.ar_fmag))) != "`\n") {
            throw std::runtime_error("Invalid file header magic in ar header");
        }
        size_t filesize = Utils::Convenient::parse_decimal(Utils::Convenient::trim(std::string(header.ar_size, sizeof(header.ar_size))));
        std::string filename;
        std::string rawName = Utils::Convenient::trim(std::string(header.ar_name, sizeof(header.ar_name)));
        LOG_DEBUG("reading object file %s, size %ld\n", rawName.c_str(), filesize);
        library_stream.read(object, filesize);
        filename = rawName;
        if (rawName == ElfMan::ArchiveObjectFile::symbol_table_name) {
            // this is a special object, Long filename string table
            symbolTable.assign(object.begin(), object.end());
        }
        else if (rawName == ElfMan::ArchiveObjectFile::name_table_name) {
            // this is a special object, Long filename string table
            nameTable.assign(object.begin(), object.end());
        }
        else if (!rawName.empty() && rawName[0] == '/' && rawName.size() > 1) {
            // Long filename reference into string table
            size_t offsetInTable = std::stoul(rawName.substr(1));
            if (offsetInTable >= nameTable.size()) {
                LOG_ERROR("Invalid string table offset %ld\n", offsetInTable);
                throw std::runtime_error("Invalid string table offset");
            }
            size_t endPos = nameTable.find('/', offsetInTable);
            if (endPos == std::string::npos) {
                endPos = nameTable.find('\n', offsetInTable);
            }
            if (endPos == std::string::npos) {
                throw std::runtime_error("Unterminated filename in string table");
            }
            filename = nameTable.substr(offsetInTable, endPos - offsetInTable);
        } 
        else {
            // Normal short name
            // don't change it
        }
        std::shared_ptr<ElfMan::ArchiveObjectFile> archive_obj = ElfMan::ArchiveObjectFile::from_bytes(object.data(), object.size(), header, filename);
        objects.push_back(archive_obj);
    }
}
//------------------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ElfMan::StaticLibrary::serialize()
{
    // archive magic bytes
    std::vector<uint8_t> data(ARMAG, ARMAG+SARMAG);

    for (auto& object : getObjects()) {
        std::vector<uint8_t> object_data = object->serialize();
        // we should check and modify object file size if needed
        LOG_DEBUG("file %s, old size %s, new size %s", object->header.ar_name, object->header.ar_size, std::to_string(object_data.size()).c_str());
        snprintf((char*)&object->header.ar_size, sizeof(object->header.ar_size), "%s", std::to_string(object_data.size()).c_str());
        memset(&((char*)&object->header.ar_size)[strlen((char*)&object->header.ar_size)], ' ', 
                                                        (sizeof(object->header.ar_size) - strlen((char*)&object->header.ar_size)));
        data.insert(data.end(), (const uint8_t*)&object->header, ((const uint8_t*)&object->header + sizeof(ar_hdr)));
        data.insert(data.end(), object_data.begin(), object_data.end());
    }

    return data;
}
//------------------------------------------------------------------------------------------------------------------------------
// Dump archive contents
void ElfMan::StaticLibrary::dump() {
    std::cout << "Archive contains " << objects.size() << " file(s):\n";
    for (auto& obj : objects) {
        std::cout << "  " << obj->filename()
                  << " (" << obj->size() << " bytes)\n";
    }
}
//------------------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ElfMan::Symbol> ElfMan::StaticLibrary::find_symbol(std::string sym_name)
{
    for (auto &object : getObjects()) {
        if (ArchiveObjectFileType::ELF_OBJECT != object->type)
            continue;
        std::shared_ptr<ObjectFile> objfile = std::dynamic_pointer_cast<ObjectFile>(object);
        if (auto search = objfile->find_symbol(sym_name))
            return search;
    }
    return nullptr;
}
//------------------------------------------------------------------------------------------------------------------------------
bool ElfMan::StaticLibrary::rename_symbol(std::string old_name, std::string new_name)
{
    bool res = false;
    for (auto &object : getObjects()) {
        if (ArchiveObjectFileType::ELF_OBJECT != object->type)
            continue;
        std::shared_ptr<ObjectFile> objfile = std::dynamic_pointer_cast<ObjectFile>(object);
        // TODO: now we assume that every symbol we modify is global, and there's no local symbols with same name
        // in other objects. This may be not always the case
        if (auto search = objfile->find_symbol(old_name) && objfile->rename_symbol(old_name, new_name))
            res = true;
    }
    return res;
}
//------------------------------------------------------------------------------------------------------------------------------
void ElfMan::StaticLibrary::reorder_symtab_and_relocations()
{
    for (auto &obj : getObjects())
        if (ElfMan::ArchiveObjectFileType::ELF_OBJECT == obj->type)
            std::dynamic_pointer_cast<ElfMan::ObjectFile>(obj)->reorder_symtab_and_relocations();
}
//------------------------------------------------------------------------------------------------------------------------------
