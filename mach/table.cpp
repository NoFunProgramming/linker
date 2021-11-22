#include "table.hpp"

#include <iostream>

namespace Mach {

/******************************************************************************/
Table::Table():
    entries(),
    strings(){}

/******************************************************************************/
void
Table::add_entry(const std::string& label)
{
    size_t idx = intern(label);
    
    Entry e;
    e.strx = (uint32_t)idx;
    e.type = 15;
    e.sect =  1;
    entries.push_back(e);
}

void
Table::add_string(const std::string& str)
{
    intern(str);
}

/******************************************************************************/
bool
Table::parse(std::ifstream& in)
{
    uint32_t sym_off;
    uint32_t sym_num;
    uint32_t str_off;
    uint32_t str_size;
    in.read((char*)&sym_off,    sizeof(sym_off));
    in.read((char*)&sym_num,    sizeof(sym_num));
    in.read((char*)&str_off,    sizeof(str_off));
    in.read((char*)&str_size,   sizeof(str_size));
    if (in.eof()) {
        std::cerr << "Error while reading table header.\n";
        return false;
    }

    size_t previous = in.tellg();

    std::cout << "Reading " << sym_num << " symbols "
              << "at " << sym_off << ".\n";
    
    in.seekg(sym_off);
    bool ok = parse_entries(in, sym_num);
    if (!ok) {
        std::cerr << "Error while reading table entries.\n";
        return false;
    }
    
    std::cout << "Reading " << str_size << " bytes "
              << "of strings at " << str_off << ".\n";
    
    in.seekg(str_off);
    ok = parse_strings(in, str_size);
    if (!ok) {
        std::cerr << "Error while reading table strings.\n";
        return false;
    }
    
    for (Entry& e : entries) {
        if (strings.count(e.strx) == 0) {
            std::cerr << "No string found for table entry.\n";
            return false;
        }
    }

    in.seekg(previous);
    return true;
}

/******************************************************************************/
void
Table::print(std::ostream& out)
{
    out << "Table:      " << "\n";
    for (auto e : entries) {
        out << "  Str Index: " << strings[e.strx]  << "\n";
        out << "    Type:      " << (int)e.type  << "\n";
        out << "    Section:   " << (int)e.sect  << "\n";
        out << "    Descript:  " << (int)e.desc  << "\n";
        out << "    Value:     " << (int)e.val   << "\n";
        out << "\n";
    }
    
    out << "Strings\n";
    for (auto s : strings) {
        out << "  " << s.second << "\n";
    }
    out << "\n";
}

/******************************************************************************/
bool
Table::write(std::ofstream& out, size_t* end)
{
    size_t previous = out.tellp();
    
    *end = ((*end + 7) >> 3) << 3;
    out.seekp(*end);

    uint32_t sym_off  = (uint32_t)out.tellp();
    uint32_t sym_num  = (uint32_t)entries.size();

    for (auto e : entries) {
        e.write(out);
    }

    std::cout << "Writing " << sym_num << " symbols "
              << "at " << sym_off << ".\n";
    
    *end = out.tellp();
    *end = ((*end + 7) >> 3) << 3;
    out.seekp(*end);
    
    uint32_t str_off  = (uint32_t)out.tellp();
    uint32_t str_size = 0;
    
    for (auto s : strings) {
        size_t len = s.second.size() + 1;
        out.write(s.second.c_str(), len);
        str_size += len;
    }
    
    std::cout << "Writing " << str_size << " bytes "
              << "of strings at " << str_off << ".\n";
    
    *end = out.tellp();
    out.seekp(previous);
    
    uint32_t cmd = 2;
    uint32_t cmd_size = write_size();
    
    out.write((char*)&cmd,      sizeof(cmd      ));
    out.write((char*)&cmd_size, sizeof(cmd_size ));
    out.write((char*)&sym_off,  sizeof(sym_off  ));
    out.write((char*)&sym_num,  sizeof(sym_num  ));
    out.write((char*)&str_off,  sizeof(str_off  ));
    out.write((char*)&str_size, sizeof(str_size ));
    
    return true;
}

uint32_t
Table::write_size() {
    return 24;
}

/******************************************************************************/
bool
Table::parse_entries(std::ifstream& in, int num)
{
    for (int i = 0; i < num; i++) {
        Entry entry;
        if (entry.read(in)) {
            entries.push_back(entry);
        } else {
            return false;
        }
    }
    return true;
}

bool
Table::parse_strings(std::ifstream& in, int size)
{
    std::streampos start = in.tellg();
    
    while (!in.eof()) {
        uint32_t idx = (uint32_t)(in.tellg() - start);
        if (idx >= size) {
            break;
        }
        std::string s;
        std::getline(in, s, '\0');
        strings[idx] = std::move(s);
    }
    if (in.eof()) {
        std::cerr << "Unexpected end of file while reading strings.\n";
        return false;
    }

    return true;
}

/******************************************************************************/
size_t
Table::intern(const std::string& label)
{
    size_t count = 0;
    for (auto s : strings) {
        if (s.second.compare(label) == 0) {
            return s.first;
        } else {
            count += s.second.size() + 1;
        }
    }
    strings[count] = label;
    return count;
}

/******************************************************************************/
Table::Entry::Entry():
    strx(0),
    type(0),
    sect(0),
    desc(0),
    val (0){}

bool
Table::Entry::read(std::ifstream& in)
{
    in.read((char*)&strx,   sizeof(strx));
    in.read((char*)&type,   sizeof(type));
    in.read((char*)&sect,   sizeof(sect));
    in.read((char*)&desc,   sizeof(desc));
    in.read((char*)&val,    sizeof(val));
    if (in.eof()) {
        std::cerr << "Unexpected end of file while reading table entry.\n";
        return false;
    }

    return true;
}

void
Table::Entry::write(std::ofstream& out)
{
    out.write((char*)&strx,   sizeof(strx));
    out.write((char*)&type,   sizeof(type));
    out.write((char*)&sect,   sizeof(sect));
    out.write((char*)&desc,   sizeof(desc));
    out.write((char*)&val,    sizeof(val));
}

}
