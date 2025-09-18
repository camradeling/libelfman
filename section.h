/*
 * Auto-added header
 * File: section.h
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#ifndef ELFMAN_SECTION_H
#define ELFMAN_SECTION_H
//------------------------------------------------------------------------------------------------------------------------------
#include <cstring>
#include <stdint.h>
#include <string>
#include <iterator>
#include <vector>
#include <algorithm>
#include <memory>
#include <map>
#include <elf.h>
//------------------------------------------------------------------------------------------------------------------------------
namespace ElfMan
{
//------------------------------------------------------------------------------------------------------------------------------
class ObjectFile;
//------------------------------------------------------------------------------------------------------------------------------
class Section {
public:
    Section(Elf32_Shdr* header, ObjectFile* obj) {
        memcpy(&shdr, header, sizeof(shdr));
        object = obj;
    }

    virtual ~Section() = default;
    virtual std::vector<uint8_t> serialize() = 0;
	std::string name();
	// getters
	uint32_t name_index() { return shdr.sh_name; }
	uint32_t offset() { return shdr.sh_offset; }
	uint32_t size() { return shdr.sh_size; }
	uint32_t link() { return shdr.sh_link; }
	uint32_t info() { return shdr.sh_info; }
	uint32_t addralign() { return shdr.sh_addralign; }
    uint32_t type() const { return shdr.sh_type; }
    const Elf32_Shdr* header() const { return &shdr; } 
	// setters
	void offset(uint32_t off) { shdr.sh_offset = off; }
	void info(uint32_t inf) { shdr.sh_info = inf; }
	void size(uint32_t sz) { shdr.sh_size = sz; }

    using FactoryFunc = std::function<std::shared_ptr<Section>(
        Elf32_Shdr*, const uint8_t*, uint32_t, ObjectFile*)>;

    // fabric method
    static std::shared_ptr<Section> from_bytes(
        Elf32_Shdr* header,
        const uint8_t* buffer,
        uint32_t total_sz,
        ObjectFile* obj);

    int index = 0;

    static void register_factory(uint32_t sh_type, FactoryFunc func);

protected:
    Elf32_Shdr shdr;
    ObjectFile* object;
private:
	static std::map<uint32_t, FactoryFunc>& registry();
};
//------------------------------------------------------------------------------------------------------------------------------
} //namespace ElfMan
//------------------------------------------------------------------------------------------------------------------------------
#endif/*ELFMAN_SECTION_H*/