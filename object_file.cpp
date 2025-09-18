/*
 * Auto-added header
 * File: object_file.cpp
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#include "memory_helpers.h"
#include "object_file.h"
#include "section.h"
#include "logger.h"
//------------------------------------------------------------------------------------------------------------------------------
const std::string ElfMan::ArchiveObjectFile::name_table_name = std::string("//");
const std::string ElfMan::ArchiveObjectFile::symbol_table_name = std::string("/");
//------------------------------------------------------------------------------------------------------------------------------
std::map<ElfMan::ArchiveObjectFileType, ElfMan::ArchiveObjectFile::FactoryFunc>& ElfMan::ArchiveObjectFile::registry() {
    static std::map<ElfMan::ArchiveObjectFileType, ElfMan::ArchiveObjectFile::FactoryFunc> instance;
    return instance;
}
//------------------------------------------------------------------------------------------------------------------------------
void ElfMan::ArchiveObjectFile::register_factory(ElfMan::ArchiveObjectFileType type, ElfMan::ArchiveObjectFile::FactoryFunc func) {
    registry()[type] = std::move(func);
}
//------------------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ElfMan::ArchiveObjectFile> ElfMan::ArchiveObjectFile::from_bytes(
	const uint8_t* buffer, 
	uint32_t total_sz, 
	struct ar_hdr hdr, 
	std::string fname)
{
	if (fname == name_table_name || fname == symbol_table_name) {
		auto it = registry().find(ArchiveObjectFileType::STRING_TABLE);
	    if (it != registry().end()) {
	        return (it->second)(buffer, total_sz, hdr, fname);
	    }
	}
    
    // fallback — binary object
    return std::make_shared<ElfMan::ObjectFile>(buffer, total_sz, hdr, fname);
}
//------------------------------------------------------------------------------------------------------------------------------
ElfMan::ObjectFile::ObjectFile(const uint8_t* buffer, uint32_t total_sz, struct ar_hdr hdr, std::string fname)
 : ArchiveObjectFile(hdr, fname)
{
	// Validation: sanity-check the provided buffer looks like an ELF object file.
	// - Ensure we have at least the ELF magic bytes and a minimal header size.
	// - Check that section header table offsets/sizes will fit within the provided buffer
	//   before attempting to read them.
	if (total_sz < 4 || buffer == nullptr) {
	    LOG_ERROR("ObjectFile: buffer is null or too small (size=%u)", total_sz);
	    throw std::runtime_error("Invalid input buffer");
	}
	if (!(buffer[0] == 0x7f && buffer[1] == 'E' && buffer[2] == 'L' && buffer[3] == 'F')) {
	    LOG_ERROR("ObjectFile: invalid ELF magic");
	    throw std::runtime_error("Not an ELF file");
	}
	// Basic check: ensure header fits
	if (total_sz < (uint32_t)sizeof(Elf32_Ehdr)) {
	    LOG_ERROR("ObjectFile: buffer smaller than Elf32_Ehdr (size=%u)", total_sz);
	    throw std::runtime_error("Truncated ELF header");
	}
	type = ArchiveObjectFileType::ELF_OBJECT;
	int index = 0;
	// we only need elf_header_stream to read elf_header
	ElfMan::Memory::InputMemoryStream elf_header_stream(buffer, total_sz);
    elf_header_stream.read(ehdr);
    ElfMan::Memory::InputMemoryStream section_header_stream(buffer+ehdr.e_shoff, ehdr.e_shentsize*ehdr.e_shnum);
    for(int i = 0; i < ehdr.e_shnum; i++) {
    	// for every new section - read it's header
    	Elf32_Shdr shdr;
    	section_header_stream.read(shdr);
    	// create a section instance, different section types are handled inside constructor
    	std::shared_ptr<ElfMan::Section> newsection = ElfMan::Section::from_bytes(&shdr,buffer + shdr.sh_offset, shdr.sh_size, this);
    	// we save all sections into a map with section offfset used as a key
    	if(newsection->size() && newsection->type() != SHT_NOBITS)
    		sections.insert(std::pair<uint32_t,std::shared_ptr<ElfMan::Section>>(shdr.sh_offset, newsection));
    	newsection->index = index++;
    	// for now we assume there's only one symbol table section
    	// this is very unlikely to be not true
    	if (SHT_SYMTAB == shdr.sh_type)
    		symtab_section = std::dynamic_pointer_cast<SymbolSection>(sections.find(shdr.sh_offset)->second);
    	// every section needs a smart pointer to it in an indexed vector
    	sections_by_index.push_back(newsection);
    	LOG_DEBUG("added section %d, size %d", newsection->index, newsection->size());
    }
    // symbol table section always have a link to symbol string table section
    symbol_strtab_section = std::dynamic_pointer_cast<RawSection>(sections_by_index[symtab_section->link()]);
    // and section name string table section is assumed to be always the last
	section_strtab_section = std::dynamic_pointer_cast<RawSection>(sections_by_index[sections_by_index.size()-1]);
	// relink all relocations
	for (auto &section_pair : sections) {
		std::shared_ptr<ElfMan::Section> section = section_pair.second;
		if(!section->size() || section->type() != SHT_REL)
			continue;
		if (auto reltab = std::dynamic_pointer_cast<RelocationSection>(section)) {
			for (auto& rel : reltab->relocations)
				rel->parent_ptr = section;
		}
		else {
			throw std::runtime_error("dynamic cast to RelocationSection failed");
		}
	}
	// populate symtab section symbol map
	// we couldn't do that until all string table sections were added
	for (auto &symbol : symtab_section->symbols) {
		symtab_section->symbols_by_name.insert(std::pair(symbol->name(), symbol));
	}
}
//------------------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ElfMan::ObjectFile::serialize()
{
	std::vector<uint8_t> object, section_header;
	object.resize(sizeof(ehdr));
	int max_size = 0;
	for (auto secpair : sections)
	{
		std::shared_ptr<ElfMan::Section> sec = secpair.second;
		if(!sec->size() || sec->type() == SHT_NOBITS)
			continue;
		std::vector<uint8_t> section_data = sec->serialize();
		// move sections if alignment requires that
		if (sec->addralign() > 1)
		{
			int padding = object.size() % sec->addralign();
			if (padding)
			{
				std::vector<uint8_t> padvec(sec->addralign() - padding, 0);
				object.insert(object.end(), padvec.begin(), padvec.end());
			}
		}
		LOG_DEBUG("changing section %d offset from 0x%08X to 0x%08lX", sec->index, sec->offset(), object.size());
		sec->offset(object.size());
		object.insert(object.end(), section_data.begin(), section_data.end());
	}
	// padding for section headers table
	int padding = object.size() % sizeof(uint32_t);
	if (padding)
	{
		std::vector<uint8_t> padvec(sizeof(uint32_t) - padding, 0);
		object.insert(object.end(), padvec.begin(), padvec.end());
	}
	// put in elf header, now that we know section header offset
	ehdr.e_shoff = object.size();
	memcpy((uint8_t*)object.data(), (uint8_t*)&ehdr, sizeof(ehdr));
	for (auto section : sections_by_index)
	{
		LOG_DEBUG("writing section header %d data size %08X, addr %08X, alignment %d\n", section->index, section->size(),
																	section->offset(), section->addralign());
		section_header.insert(section_header.end(), (char*)section->header(), (char*)section->header() + sizeof(Elf32_Shdr));
	}
	object.insert(object.end(), section_header.begin(), section_header.end());
	return object;
}
//------------------------------------------------------------------------------------------------------------------------------
void ElfMan::ObjectFile::move_section_offsets(uint32_t addr, int addend)
{
	// move all sections offsets higher than the addr of modified one
	for (auto secpair : sections)
	{
		std::shared_ptr<ElfMan::Section> sec = secpair.second;
		if (sec->offset() < addr)
			continue;
		// adjust alignment
		sec->offset(sec->offset() + addend);
		LOG_DEBUG("moving section %d from 0x%08X to 0x%08X\n", sec->index, sec->offset(), sec->offset() + addend);
		uint32_t offset = sec->offset();
	}
}
//------------------------------------------------------------------------------------------------------------------------------
// when we changed one or few symbols visibility (local to global or vice versa), we need to reorder them in a symbol table,
// and after that we need to move all relocations, so that they pointed to correct symbol indexes
// we take symbols by smart pointer, put them into new vector, change their indexes, but their position in original vector is equal to their old index.
// so we always know both new and old indexes, and after correction just swap vectors
void ElfMan::ObjectFile::reorder_symtab_and_relocations()
{
	// reorder symbol table, keeping old indexes inside
	std::vector<std::shared_ptr<ElfMan::Symbol>> global_symbols, local_symbols, replacement;
	for (auto symbol : symtab_section->symbols)
	{
		if (symbol->bind() == STB_GLOBAL)
			global_symbols.push_back(symbol);
		else
			local_symbols.push_back(symbol);
	}
	replacement.insert(replacement.end(), local_symbols.begin(), local_symbols.end());
	replacement.insert(replacement.end(), global_symbols.begin(), global_symbols.end());
	// rewrite all symbols symtab indexes
	for (int i = 0; i < replacement.size(); i++)
	{
		replacement[i]->index = i;
	}
	// move symtab section first global symbol index
	symtab_section->info(local_symbols.size());
	// now relink all relocations to new symtab indexes
	for (auto section : sections_by_index)
	{
		if (section->type() != SHT_REL)
			continue;
		if (auto reltab = std::dynamic_pointer_cast<RelocationSection>(section)) {
			for (auto& rel : reltab->relocations)
			{
				LOG_DEBUG("check relocation %d, offset 0x%08X", rel->index, rel->rhdr.r_offset);
				// symtab->symbols is not yet reordered,
				// but all symbol instances in it already have indexes from replacement collection
				// relocation instances still point to old indexes in symtab
				// so we take index from relocation instance, dereference it to a symbol instance
				// then check if its new index matches the old one.
				// and if not - just rewrite index in relocation instance
				std::shared_ptr<ElfMan::Symbol> sym = rel->symbol_to_apply();
				if (!sym)
				{
					LOG_ERROR("failed to find symbol for relocation %d\n", rel->index);
					continue;
				}
				if (sym->index != ELF32_R_SYM(rel->rhdr.r_info))
				{
					LOG_DEBUG("inconsistent relocation entry for symbol %s, remapping %d to %d\n", 
																				sym->name().c_str(),
																				ELF32_R_SYM(rel->rhdr.r_info),
																				sym->index);
					rel->rhdr.r_info = ELF32_R_INFO(sym->index,ELF32_R_TYPE(rel->rhdr.r_info));
				}
			}
		}
		else {
			throw std::runtime_error("dynamic cast to RelocationSection failed");
		}
	}
	std::swap(symtab_section->symbols,replacement);
}
//------------------------------------------------------------------------------------------------------------------------------
// this method rewrite all relocation instancess pointing to a symbol so that they started pointing to another symbol
void ElfMan::ObjectFile::move_relocations(int src_ind, int dest_ind)
{
	for (auto section : sections_by_index)
	{
		if (section->type() != SHT_REL)
			continue;
		if (auto relsection = std::dynamic_pointer_cast<ElfMan::RelocationSection>(section)) {
			for (auto& rel : relsection->relocations) {
				LOG_DEBUG("check relocation %d, offset 0x%08X\n", rel->index, rel->rhdr.r_offset);
				std::shared_ptr<ElfMan::Symbol> sym = rel->symbol_to_apply();
				if (!sym)
				{
					LOG_ERROR("failed to find symbol for relocation %d\n", rel->index);
					continue;
				}
				if (sym->index == src_ind)
				{
					LOG_INFO("found relocation entry for symbol %d, remapping to symbol %d, section %d,\n", 
																				src_ind,
																				dest_ind,
																				section->index);
					rel->rhdr.r_info = ELF32_R_INFO(dest_ind,ELF32_R_TYPE(rel->rhdr.r_info));
				}
			}
		}
		else {
			throw std::runtime_error("dynamic cast to RelocationSection failed");
		}
	}
}
//------------------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ElfMan::Symbol> ElfMan::ObjectFile::insert_undefined_global_function(std::string name, bool thumb)
{
	// insert new symbol
	Elf32_Sym symhdr;
	symhdr.st_value = thumb ? 1 : 0;
	symhdr.st_size = 0;
	symhdr.st_other = STV_DEFAULT;
	symhdr.st_shndx = SHN_UNDEF;
	// insert new symbol name to strtab
	std::shared_ptr<ElfMan::Symbol> newsym(new ElfMan::Symbol(&symhdr, this));
	// last index offset in strtab will be current strtab size - we write to the end
	newsym->symhdr.st_name = symbol_strtab_section->data.size();
	newsym->symhdr.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_FUNC);
	symbol_strtab_section->data.resize(symbol_strtab_section->data.size()+name.size()+1);
	std::fill(symbol_strtab_section->data.begin()+newsym->symhdr.st_name, symbol_strtab_section->data.end(),0);
	// writning new symbol name into last index offset in strtab
	memcpy(&symbol_strtab_section->data.data()[newsym->symhdr.st_name], name.data(), name.size());
	// now saving new symbol index
	newsym->index = symtab_section->symbols.size();
	// and saving symbol instance
	symtab_section->symbols.push_back(newsym);
	symtab_section->symbols_by_name.insert(std::pair(newsym->name(), newsym));
	// updating section sizes
	symtab_section->size(symtab_section->size() + sizeof(Elf32_Sym));
	symbol_strtab_section->size(symbol_strtab_section->size() + name.size()+1);
	LOG_DEBUG("%s %08X %08X %08X %02X %02X %08X\n", newsym->name().c_str(),
													newsym->symhdr.st_name,
													newsym->symhdr.st_value,
													newsym->symhdr.st_size,
													newsym->symhdr.st_info,
													newsym->symhdr.st_other,
													newsym->symhdr.st_shndx);
	return std::move(newsym);
}
//------------------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ElfMan::Symbol> ElfMan::ObjectFile::find_symbol(std::string sym_name)
{
	if (auto search = symtab_section->symbols_by_name.find(sym_name); search != symtab_section->symbols_by_name.end())
		return search->second;
	else
		return nullptr;
}
//------------------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ElfMan::Symbol> ElfMan::ObjectFile::rename_symbol(std::string old_name, std::string new_name)
{
	auto sympair = symtab_section->symbols_by_name.find(old_name);
	if (sympair == symtab_section->symbols_by_name.end()) {
		LOG_ERROR("symbol %s not found", old_name.c_str());
		return nullptr;
	}
	std::shared_ptr<ElfMan::Symbol> symbol = sympair->second;
	if (!symbol) {
		LOG_ERROR("symbol %s not found", old_name.c_str());
		return nullptr;
	}
	// change name for symbol
	uint8_t* ptr = &symbol_strtab_section->data.data()[symbol->symhdr.st_name];
	memset((char *)ptr, 0, old_name.size());
	strncpy((char *)ptr, new_name.data(), old_name.size());
	return symbol;
}
//------------------------------------------------------------------------------------------------------------------------------
bool ElfMan::StringTable::registered = []{
    ArchiveObjectFile::register_factory(ElfMan::ArchiveObjectFileType::STRING_TABLE,
        [](const uint8_t* b, uint32_t sz, struct ar_hdr h, std::string f) {
            return std::make_shared<ElfMan::StringTable>(b, sz, h, f);
        });
    return true;
}();
//------------------------------------------------------------------------------------------------------------------------------
bool ElfMan::ObjectFile::registered = []{
    ArchiveObjectFile::register_factory(ElfMan::ArchiveObjectFileType::ELF_OBJECT,
        [](const uint8_t* b, uint32_t sz, struct ar_hdr h, std::string f) {
            return std::make_shared<ElfMan::ObjectFile>(b, sz, h, f);
        });
    return true;
}();
//------------------------------------------------------------------------------------------------------------------------------
