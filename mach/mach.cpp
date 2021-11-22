#include "mach.hpp"

#include <iostream>

namespace Mach
{

/******************************************************************************/
Mach::Mach():
    magic       (0xFEEDFACF),
    cpu_type    (0),
    cpu_subtype (0),
    file_type   (1),
    flags       (0){}

bool
Mach::parse(std::ifstream& in)
{
    uint32_t cmd_num;
    uint32_t cmd_size;
    uint32_t reserved;

    in.read((char*)&magic,       sizeof(magic));
    in.read((char*)&cpu_type,    sizeof(cpu_type));
    in.read((char*)&cpu_subtype, sizeof(cpu_subtype));
    in.read((char*)&file_type,   sizeof(file_type));
    in.read((char*)&cmd_num,     sizeof(cmd_num));
    in.read((char*)&cmd_size,    sizeof(cmd_size));
    in.read((char*)&flags,       sizeof(flags));
    in.read((char*)&reserved,    sizeof(reserved));
    if (in.eof()) {
        std::cerr << "Error while reading header.\n";
        return false;
    }
    
    for (uint32_t i = 0; i < cmd_num; i++)
    {
        uint32_t cmd;
        uint32_t cmd_size;
        in.read((char*)&cmd, sizeof(cmd));
        in.read((char*)&cmd_size, sizeof(cmd_size));
        if (in.eof()) {
            std::cerr << "Error while reading command.\n";
            return false;
        }
        
        size_t previous = in.tellg();
        
        switch (cmd) {
            case 0x02: {
                std::unique_ptr<Table> table = std::make_unique<Table>();
                if (table->parse(in)) {
                    tables.push_back(std::move(table));
                } else {
                    std::cerr << "Error while reading symbol table.\n";
                    return false;
                }
                break;
            }
            case 0x19: {
                std::unique_ptr<Segment> seg = std::make_unique<Segment>();
                if (seg->parse(in)) {
                    segments.push_back(std::move(seg));
                } else {
                    std::cerr << "Error while reading segment command.\n";
                    return false;
                }
                break;
            }
            default: {
                std::cout << "Unknown Command.\n";
                break;
            }
        }
        
        in.seekg(previous + cmd_size - 8);
    }
    
    std::cout << "\n";

    return true;
}

void
Mach::print(std::ostream& out)
{
    out << std::hex << std::showbase;
    out << "Magic:        " << magic        << "\n";
    out << "CPU Type:     " << cpu_type     << "\n";
    out << "CPU Subtype:  " << cpu_subtype  << "\n";
    
    out << std::dec;
    out << "File Type:    " << file_type    << "\n";
    out << "Flags:        " << flags        << "\n";
    out << "\n";
    
    for (auto& s : segments) {
        s->print(out);
    }
    for (auto& t : tables) {
        t->print(out);
    }
}

void
Mach::write(std::ofstream& out)
{
    uint32_t cmd_num  = 0;
    uint32_t cmd_size = 0;
    uint32_t reserved = 0;

    for (auto& s : segments) {
        cmd_num++;
        cmd_size += s->cmd_size();
    }
    for (auto& t : tables) {
        cmd_num++;
        cmd_size += t->write_size();
    }
    
    out.write((char*)&magic,       sizeof(magic));
    out.write((char*)&cpu_type,    sizeof(cpu_type));
    out.write((char*)&cpu_subtype, sizeof(cpu_subtype));
    out.write((char*)&file_type,   sizeof(file_type));
    out.write((char*)&cmd_num,     sizeof(cmd_num));
    out.write((char*)&cmd_size,    sizeof(cmd_size));
    out.write((char*)&flags,       sizeof(flags));
    out.write((char*)&reserved,    sizeof(reserved));
    
    size_t head = out.tellp();
    size_t end  = head + cmd_size;
    
    for (auto& s : segments) {
        out.seekp(head);
        s->write(out, &end);
        head += s->cmd_size();
    }
    
    for (auto& t : tables) {
        out.seekp(head);
        t->write(out, &end);
        head += t->write_size();
    }
    
    std::cout << "\n";
}

}
