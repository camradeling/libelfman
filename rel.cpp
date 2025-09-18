/*
 * Auto-added header
 * File: rel.cpp
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#include <cstring>
#include "memory_helpers.h"
#include "section.h"
#include "symbol.h"
#include "object_file.h"
#include "rel.h"
#include "logger.h"
//------------------------------------------------------------------------------------------------------------------------------
ElfMan::Rel::Rel(Elf32_Rel* header, ElfMan::ObjectFile* obj)
{
	rhdr = *header;
	object = obj;
}
//------------------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ElfMan::Rel::serialize()
{
	std::vector<uint8_t> result;
	result.insert(result.end(), (char*)&rhdr, (char*)&rhdr+sizeof(rhdr));
	return result;
}
//------------------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ElfMan::Section> ElfMan::Rel::section_to_modify()
{
	std::shared_ptr<ElfMan::Section> parent = parent_ptr.lock();
	if (parent)
		return object->sections_by_index[parent->info()];
	else
		return nullptr;
}
//------------------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ElfMan::Symbol> ElfMan::Rel::symbol_to_apply() // Note! search by index!
{
	std::shared_ptr<ElfMan::Section> parent = parent_ptr.lock();
	if (parent)
	{
		LOG_DEBUG("\tparent section is %d, link section id %d, symbol offset %d", parent->index, parent->link(),
																	ELF32_R_SYM(rhdr.r_info));
		std::shared_ptr<SymbolSection> symtab = std::dynamic_pointer_cast<SymbolSection>(object->sections_by_index[parent->link()]);
		return symtab->symbols[ELF32_R_SYM(rhdr.r_info)];
	}
	else
	{
		return nullptr;
	}
}
//------------------------------------------------------------------------------------------------------------------------------
