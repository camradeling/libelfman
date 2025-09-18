/*
 * Auto-added header
 * File: raw_section.h
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#ifndef RAW_SECTION_H
#define RAW_SECTION_H
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
//------------------------------------------------------------------------------------------------------------------------------
namespace ElfMan
{
//------------------------------------------------------------------------------------------------------------------------------
class RawSection : public Section {
public:
    RawSection(Elf32_Shdr* header, const uint8_t* buffer, uint32_t total_sz, ObjectFile* obj)
        : Section(header, obj), data(total_sz)
    {
        memcpy(data.data(), buffer, total_sz);
    }
    virtual std::vector<uint8_t> serialize() {
        return data;
    }
    std::vector<uint8_t> data;
};
//------------------------------------------------------------------------------------------------------------------------------
} //namespace ElfMan
//------------------------------------------------------------------------------------------------------------------------------
#endif/*RAW_SECTION_H*/
