#include "segment.hpp"
//#include "debug.hpp"

#include <iostream>

using std::unique_ptr;
using std::make_unique;

namespace Mach
{

/******************************************************************************/
Segment::Segment():
    name        (),
    protect_max (7),
    protect_init(7),
    flags       (0)
    {}

bool
Segment::parse(std::ifstream& in)
{
    std::getline(in, name, '\0');
    if (name.size() < 16) {
        in.seekg(15 - name.size(), std::ios::cur);
    } else {
        std::cerr << "Segment name is too long.\n";
        return false;
    }
    
    uint64_t mem_size;
    uint64_t file_off;
    uint64_t file_size;
    uint32_t num_sect;
    
    in.read((char*)&addr,           sizeof(addr));
    in.read((char*)&mem_size,       sizeof(mem_size));
    in.read((char*)&file_off,       sizeof(file_off));
    in.read((char*)&file_size,      sizeof(file_size));
    in.read((char*)&protect_max,    sizeof(protect_max));
    in.read((char*)&protect_init,   sizeof(protect_init));
    in.read((char*)&num_sect,       sizeof(num_sect));
    in.read((char*)&flags,          sizeof(flags));
    if (in.eof()) {
        std::cerr << "Unexpected end of file in segment.\n";
        return false;
    }
    
    for (uint32_t i = 0; i < num_sect; i++)
    {
        unique_ptr<Section> sect = make_section(in);
        if (!sect) {
            return false;
        }
        
        if (sect->parse(in)) {
            sections.push_back(std::move(sect));
        } else {
            std::cerr << "Error while reading section.\n";
            return false;
        }
    }
    
    return true;
}

unique_ptr<Section>
Segment::make_section(std::ifstream& in)
{
    std::string name;
    std::getline(in, name, '\0');
    if (name.size() < 16) {
        in.seekg(16 - name.size() - 1, std::ios::cur);
    } else {
        std::cerr << "Section name is too long.\n";
        return nullptr;
    }
    
    std::string segment;
    std::getline(in, segment, '\0');
    if (segment.size() < 16) {
        in.seekg(16 - segment.size() - 1, std::ios::cur);
    } else {
        std::cerr << "Segment name is too long.\n";
        return nullptr;
    }
    in.seekg(-32, std::ios::cur);
    
    unique_ptr<Section> sect;
//    if ((name.compare("__debug_line") == 0) &&
//        (segment.compare("__DWARF") == 0)) {
//        sect = std::make_unique<Debug_Lines>();
//    } else {
//        sect = std::make_unique<Section>();
//    }
    sect = std::make_unique<Section>();

    return sect;
}

void
Segment::print(std::ostream& out)
{
    out << "Segment:        " << name            << "\n";
    out << "  Address:      " << addr            << "\n";
    out << "  Protect Max:  " << protect_max     << "\n";
    out << "  Protect Init: " << protect_init    << "\n";
    out << "  Num Sections: " << sections.size() << "\n";
    out << "  Flags:        " << flags           << "\n";
    out << "  \n";
    
    for (auto& s : sections) {
        s->print(out);
    }
}

bool
Segment::write(std::ofstream& out, size_t* end)
{
    size_t previous = out.tellp();
    out.seekp(72, std::ios::cur);

    uint64_t start = *end;
    for (auto& s : sections) {
        bool ok = s->write(out, end);
        if (!ok) {
            return false;
        }
    }

    out.seekp(previous);

    uint32_t cmd  = 0x19;
    uint32_t size = cmd_size();
    
    out.write((char*)&cmd,  sizeof(cmd));
    out.write((char*)&size, sizeof(size));
        
    if (name.size() < 16) {
        out.write(name.c_str(), name.size());
        for (size_t i = name.size(); i < 16; i++) {
            out.put('\0');
        }
    } else {
        std::cerr << "Segment name is too long.\n";
        return false;
    }
    
    // TODO Calculate by asking the sections.
    uint64_t mem_size  = *end - start;
    uint64_t file_off  = start;
    uint64_t file_size = *end - start;
    uint32_t num_sect  = (uint32_t)sections.size();

    out.write((char*)&addr,         sizeof(addr));
    out.write((char*)&mem_size,     sizeof(mem_size));
    out.write((char*)&file_off,     sizeof(file_off));
    out.write((char*)&file_size,    sizeof(file_size));
    out.write((char*)&protect_max,  sizeof(protect_max));
    out.write((char*)&protect_init, sizeof(protect_init));
    out.write((char*)&num_sect,     sizeof(num_sect));
    out.write((char*)&flags,        sizeof(flags));
    
    return true;
}

uint32_t
Segment::cmd_size()
{
    uint32_t result = 72;
    for (auto& s : sections) {
        result += s->cmd_size();
    }
    return result;
}

}
