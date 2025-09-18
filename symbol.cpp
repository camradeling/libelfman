/*
 * Auto-added header
 * File: symbol.cpp
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#include <cstring>
#include "symbol.h"
#include "object_file.h"
//------------------------------------------------------------------------------------------------------------------------------
ElfMan::Symbol::Symbol(Elf32_Sym* sym, ElfMan::ObjectFile* obj)
{
	memcpy(&symhdr,sym,sizeof(symhdr));
	object = obj;
}
//------------------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ElfMan::Symbol::serialize()
{
	return data;
}
//------------------------------------------------------------------------------------------------------------------------------
std::string ElfMan::Symbol::name()
{
	if(!object)
		return "*error1*";
	std::shared_ptr<RawSection> strtab;
	if (ELF32_ST_TYPE(symhdr.st_info) == STT_SECTION)
	{
		LOG_DEBUG("retrieving section name");
		strtab = object->section_strtab_section;
		if (!strtab)
		{
			LOG_DEBUG("undefinded section header string table");
			exit(-1);
		}
		LOG_DEBUG("section size = %ld", strtab->data.size());
		int sec_name_index = object->sections_by_index[symhdr.st_shndx]->name_index();
		LOG_DEBUG("section name index = %d", sec_name_index);
		return std::string((char*)&strtab->data.data()[sec_name_index]);
	}
	strtab = object->symbol_strtab_section;
	if(!strtab)
		return "*error2*";
	if (symhdr.st_name >= strtab->size())
		return "*error3*";
	LOG_DEBUG("getting name %s", (char*)&strtab->data.data()[symhdr.st_name]);
	return std::string((char*)&strtab->data.data()[symhdr.st_name]);
}//------------------------------------------------------------------------------------------------------------------------------
uint32_t ElfMan::Symbol::offset()
{
	return symhdr.st_value;
}
//------------------------------------------------------------------------------------------------------------------------------
int ElfMan::Symbol::bind()
{
	return ELF32_ST_BIND(symhdr.st_info);
}
//------------------------------------------------------------------------------------------------------------------------------
bool ElfMan::Symbol::set_global()
{
	symhdr.st_info = ELF32_ST_INFO(STB_GLOBAL,ELF32_ST_TYPE(symhdr.st_info));
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------