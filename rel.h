/*
 * Auto-added header
 * File: rel.h
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#ifndef ELFMAN_REL_H
#define ELFMAN_REL_H
//------------------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <string>
#include <iterator>
#include <vector>
#include <algorithm>
#include <memory>
#include <elf.h>
//------------------------------------------------------------------------------------------------------------------------------
namespace ElfMan
{
//------------------------------------------------------------------------------------------------------------------------------
class ObjectFile;
class Symbol;
//------------------------------------------------------------------------------------------------------------------------------
class Rel
{
public:
	Rel(Elf32_Rel* header, ElfMan::ObjectFile* obj);
	std::vector<uint8_t> serialize();
	Elf32_Rel rhdr;
	std::weak_ptr<Section> parent_ptr;
	int index = 0;
	ObjectFile* object;
	std::shared_ptr<Section> section_to_modify();
	std::shared_ptr<Symbol> symbol_to_apply();
};
//------------------------------------------------------------------------------------------------------------------------------
}//namespace ElfMan
//------------------------------------------------------------------------------------------------------------------------------
#endif/*ELFMAN_REL_H*/