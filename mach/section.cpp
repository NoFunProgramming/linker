#include "section.hpp"

#include <iostream>

namespace Mach
{

/******************************************************************************/
Section::Section():
    name        (),
    segment     (),
    addr        (0),
    align       (0),
    flags       (0x80000400){}

bool
Section::parse(std::ifstream& in)
{
    std::getline(in, name, '\0');
    if (name.size() < 16) {
        in.seekg(16 - name.size() - 1, std::ios::cur);
    } else {
        std::cerr << "Section name is too long.\n";
        return false;
    }
    
    std::getline(in, segment, '\0');
    if (segment.size() < 16) {
        in.seekg(16 - segment.size() - 1, std::ios::cur);
    } else {
        std::cerr << "Segment name is too long.\n";
        return false;
    }
    
    uint64_t size;
    uint32_t offset;
    uint32_t relative;
    uint32_t num_rel;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;

    in.read((char*)&addr,       sizeof(addr));
    in.read((char*)&size,       sizeof(size));
    in.read((char*)&offset,     sizeof(offset));
    in.read((char*)&align,      sizeof(align));
    in.read((char*)&relative,   sizeof(relative));
    in.read((char*)&num_rel,    sizeof(num_rel));
    in.read((char*)&flags,      sizeof(flags));
    in.read((char*)&reserved1,  sizeof(reserved1));
    in.read((char*)&reserved2,  sizeof(reserved2));
    in.read((char*)&reserved3,  sizeof(reserved3));
    if (in.eof()) {
        std::cerr << "Unexpected end of file.\n";
        return false;
    }
        
    std::cout << "Reading " << size << " bytes "
              << "of section data at " << offset << ".\n";
    
    size_t previous = in.tellg();
    in.seekg(offset);
    parse_data(in, size);
    in.seekg(previous);

    return true;
}

bool
Section::parse_data(std::ifstream& in, size_t size)
{
    data.resize(size);
    in.read((char*)data.data(), size);
    if (in.eof()) {
        std::cerr << "Unexpected end of file in section data.\n";
        return false;
    }
    return true;
}

void
Section::print(std::ostream& out)
{
    out << "  Section:   " << name      << "\n";
    out << "    Segment: " << segment   << "\n";
    out << "    Address: " << addr      << "\n";
    out << "    Align:   " << align     << "\n";
    
    out << std::hex << std::showbase;
    out << "    Flags:   " << flags     << "\n";
    out << std::dec;
    out << "\n";

    print_data(out);
    
    out << std::dec;
    out << "\n";
    out << "\n";
}

bool
Section::print_data(std::ostream& out)
{
    out << "    Data: ";
    out << std::hex;
    for (int i = 0; i < data.size(); i++) {
        out << (int)data[i] << " " ;
    }
    return true;
}


bool
Section::write(std::ofstream& out, size_t* end)
{
    size_t previous = out.tellp();

    uint32_t offset = (uint32_t)*end;
    uint64_t size   = data.size();

    out.seekp(offset);
    write_data(out);
    *end = out.tellp();

    std::cout << "Writing " << size << " bytes "
              << "of section data at " << offset << ".\n";
    
    out.seekp(previous);
    
    if (name.size() < 16) {
        out.write(name.c_str(), name.size());
        for (size_t i = name.size(); i < 16; i++) {
            out.put('\0');
        }
    } else {
        std::cerr << "Section name is too long.\n";
        return false;
    }
    
    if (segment.size() < 16) {
        out.write(segment.c_str(), segment.size());
        for (size_t i = segment.size(); i < 16; i++) {
            out.put('\0');
        }
    } else {
        std::cerr << "Segment name is to o long.\n";
        return false;
    }
    
    uint32_t relative = 0;
    uint32_t num_rel  = 0;
    uint32_t reserved1 = 0;
    uint32_t reserved2 = 0;
    uint32_t reserved3 = 0;
                
    out.write((char*)&addr,     sizeof(addr));
    out.write((char*)&size,     sizeof(size));
    out.write((char*)&offset,   sizeof(offset));
    out.write((char*)&align,    sizeof(align));
    out.write((char*)&relative, sizeof(relative));
    out.write((char*)&num_rel,  sizeof(num_rel));
    out.write((char*)&flags,    sizeof(flags));
    out.write((char*)&reserved1, sizeof(reserved1));
    out.write((char*)&reserved2, sizeof(reserved2));
    out.write((char*)&reserved3, sizeof(reserved3));
    
    return true;
}

bool
Section::write_data(std::ofstream& out)
{
    out.write((char*)data.data(), data.size());

    return true;
}

uint32_t Section::cmd_size() { return 80; }

}
