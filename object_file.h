/*
 * Auto-added header
 * File: object_file.h
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#ifndef ELFMAN_OBJECT_FILE_H
#define ELFMAN_OBJECT_FILE_H
//------------------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <string>
#include <iterator>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <elf.h>
#include <ar.h>
//------------------------------------------------------------------------------------------------------------------------------
#include "section.h"
#include "symbol.h"
#include "rel.h"
#include "symbol_section.h"
#include "relocation_section.h"
#include "raw_section.h"
#include "convenient.h"
//------------------------------------------------------------------------------------------------------------------------------
namespace ElfMan
{
//------------------------------------------------------------------------------------------------------------------------------
enum ArchiveObjectFileType {
	UNKNOWN,
	STRING_TABLE,
	ELF_OBJECT,
};
//------------------------------------------------------------------------------------------------------------------------------
class ArchiveObjectFile {
public:
	ArchiveObjectFile(struct ar_hdr hdr, std::string fname)
	: header(hdr),_filename(fname) { type = ArchiveObjectFileType::UNKNOWN; }
	virtual ~ArchiveObjectFile() = default;
	virtual std::vector<uint8_t> serialize() = 0;

	using FactoryFunc = std::function<std::shared_ptr<ArchiveObjectFile>(
        const uint8_t* buffer, uint32_t total_sz, struct ar_hdr hdr, std::string fname)>;

	// fabric method
    static std::shared_ptr<ArchiveObjectFile> from_bytes(const uint8_t* buffer, uint32_t total_sz, struct ar_hdr hdr, std::string fname);
    std::string filename() { return std::string(_filename); }
    size_t size() { return Utils::Convenient::parse_decimal(Utils::Convenient::trim(std::string(header.ar_size))); }
    static void register_factory(ElfMan::ArchiveObjectFileType type, ElfMan::ArchiveObjectFile::FactoryFunc func);

	struct ar_hdr header; // archive header
	static const std::string name_table_name;
    static const std::string symbol_table_name;
    ArchiveObjectFileType type;
private:
	std::string _filename;
	static std::map<ElfMan::ArchiveObjectFileType, FactoryFunc>& registry();
};
//------------------------------------------------------------------------------------------------------------------------------
class StringTable : public ArchiveObjectFile
{
public:
	StringTable(const uint8_t* buffer, uint32_t total_sz, struct ar_hdr hdr, std::string fname)
	: data(buffer, buffer + total_sz), ArchiveObjectFile(hdr, fname) { type = ArchiveObjectFileType::STRING_TABLE; }
	virtual ~StringTable(){}
	virtual std::vector<uint8_t> serialize() { return data; };
	std::vector<uint8_t> data;
private:
	static bool registered;
};
//------------------------------------------------------------------------------------------------------------------------------
class ObjectFile : public ArchiveObjectFile
{
public:
	ObjectFile(const uint8_t* buffer, uint32_t total_sz, struct ar_hdr hdr, std::string fname);
	virtual std::vector<uint8_t> serialize();
	void move_section_offsets(uint32_t addr, int addend);
	void reorder_symtab_and_relocations();
	void move_relocations(int src_ind, int dest_ind);
	std::shared_ptr<ElfMan::Symbol> insert_undefined_global_function(std::string name, bool thumb);
	std::shared_ptr<ElfMan::Symbol> find_symbol(std::string sym_name);
	std::shared_ptr<ElfMan::Symbol> rename_symbol(std::string old_name, std::string new_name);
public:
	Elf32_Ehdr ehdr; // ELF file header
	std::vector<std::shared_ptr<Section>> sections_by_index;
	std::map<uint32_t, std::shared_ptr<Section>> sections; 
	std::shared_ptr<SymbolSection> symtab_section;
	std::shared_ptr<RawSection> symbol_strtab_section;
	std::shared_ptr<RawSection> section_strtab_section;
private:
	static bool registered;
};
//------------------------------------------------------------------------------------------------------------------------------
}//namespace ElfMan
//------------------------------------------------------------------------------------------------------------------------------
#endif/*ELFMAN_OBJECT_FILE_H*/