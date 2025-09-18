/*
 * Auto-added header
 * File: symbol_section.h
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#ifndef SYMBOL_SECTION_H
#define SYMBOL_SECTION_H
//------------------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <string>
#include <iterator>
#include <vector>
#include <algorithm>
#include <memory>
#include <elf.h>
//------------------------------------------------------------------------------------------------------------------------------
#include "section.h"
#include "symbol.h"
#include "memory_helpers.h"
//------------------------------------------------------------------------------------------------------------------------------
namespace ElfMan
{
//------------------------------------------------------------------------------------------------------------------------------
class SymbolSection : public Section {
public:
    SymbolSection(Elf32_Shdr* header, const uint8_t* buffer, uint32_t total_sz, ObjectFile* obj)
        : Section(header, obj)
    {
        ElfMan::Memory::InputMemoryStream stream(buffer, total_sz);
        int index = 0;
        while (stream.size()) {
            Elf32_Sym symhdr;
            stream.read(symhdr);
            auto symbol = std::make_shared<ElfMan::Symbol>(&symhdr, obj);
            symbols.push_back(symbol);
            symbol->index = index++;
        }
    }
    virtual std::vector<uint8_t> serialize() {
        std::vector<uint8_t> result;
        for (auto sym : symbols)
        {
            result.insert(result.end(), (char*)&sym->symhdr, (char*)&sym->symhdr+sizeof(sym->symhdr));
        }
        return result;
    }
    std::vector<std::shared_ptr<ElfMan::Symbol>> symbols;
    std::map<std::string, std::shared_ptr<ElfMan::Symbol>> symbols_by_name;
private:
    static bool registered;
};
//------------------------------------------------------------------------------------------------------------------------------
} //namespace ElfMan
//------------------------------------------------------------------------------------------------------------------------------
#endif/*SYMBOL_SECTION_H*/
