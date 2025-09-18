/*
 * Auto-added header
 * File: relocation_section.h
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#ifndef RELOCATION_SECTION_H
#define RELOCATION_SECTION_H
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
#include "logger.h"
#include "rel.h"
#include "memory_helpers.h"
//------------------------------------------------------------------------------------------------------------------------------
namespace ElfMan
{
//------------------------------------------------------------------------------------------------------------------------------
class RelocationSection : public Section {
public:
    RelocationSection(Elf32_Shdr* header, const uint8_t* buffer, uint32_t total_sz, ObjectFile* obj)
        : Section(header, obj)
    {
        ElfMan::Memory::InputMemoryStream stream(buffer, total_sz);
        int index = 0;
        while (stream.size()) {
            Elf32_Rel rhdr;
            stream.read(rhdr);
            LOG_DEBUG("found relocation r_offset 0x%08X, r_info 0x%08X", rhdr.r_offset, rhdr.r_info);
            auto rel = std::make_shared<ElfMan::Rel>(&rhdr, obj);
            relocations.push_back(rel);
            rel->index = index++;
        }
    }
    virtual std::vector<uint8_t> serialize() {
        std::vector<uint8_t> result;
        for (auto rel : relocations)
        {
            result.insert(result.end(), (char*)&rel->rhdr, (char*)&rel->rhdr+sizeof(rel->rhdr));
        }
        return result;
    }
    std::vector<std::shared_ptr<ElfMan::Rel>> relocations;

private:
    static bool registered;
};
//------------------------------------------------------------------------------------------------------------------------------
} //namespace ElfMan
//------------------------------------------------------------------------------------------------------------------------------
#endif/*RELOCATION_SECTION_H*/
