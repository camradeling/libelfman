/*
 * Auto-added header
 * File: section.cpp
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#include <cstring>
//------------------------------------------------------------------------------------------------------------------------------
#include "memory_helpers.h"
#include "section.h"
#include "symbol_section.h"
#include "relocation_section.h"
#include "symbol.h"
#include "rel.h"
#include "object_file.h"
#include "logger.h"
//------------------------------------------------------------------------------------------------------------------------------
std::map<uint32_t, ElfMan::Section::FactoryFunc>& ElfMan::Section::registry() {
    static std::map<uint32_t, ElfMan::Section::FactoryFunc> instance;
    return instance;
}
//------------------------------------------------------------------------------------------------------------------------------
void ElfMan::Section::register_factory(uint32_t sh_type, ElfMan::Section::FactoryFunc func) {
    registry()[sh_type] = std::move(func);
}
//------------------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ElfMan::Section> ElfMan::Section::from_bytes(
    Elf32_Shdr* header,
    const uint8_t* buffer,
    uint32_t total_sz,
    ObjectFile* obj)
{
    auto it = registry().find(header->sh_type);
    if (it != registry().end()) {
        return (it->second)(header, buffer, total_sz, obj);
    }
    // fallback — raw section
    return std::make_shared<RawSection>(header, buffer, total_sz, obj);
}
//------------------------------------------------------------------------------------------------------------------------------
std::string ElfMan::Section::name()
{
	if(!object)
		return "*error1*";
	std::shared_ptr<RawSection> strtab = object->section_strtab_section;
	if(!strtab || shdr.sh_name >= strtab->shdr.sh_size)
		return "*error2*";
	return std::string((char*)&strtab->data.data()[shdr.sh_name]);
}
//------------------------------------------------------------------------------------------------------------------------------
bool ElfMan::RelocationSection::registered = []{
    Section::register_factory(SHT_REL,
        [](Elf32_Shdr* h, const uint8_t* b, uint32_t sz, ObjectFile* o) {
            return std::make_shared<RelocationSection>(h, b, sz, o);
        });
    return true;
}();
//------------------------------------------------------------------------------------------------------------------------------
bool ElfMan::SymbolSection::registered = []{
    Section::register_factory(SHT_SYMTAB,
        [](Elf32_Shdr* h, const uint8_t* b, uint32_t sz, ObjectFile* o) {
            return std::make_shared<SymbolSection>(h, b, sz, o);
        });
    return true;
}();
//------------------------------------------------------------------------------------------------------------------------------
