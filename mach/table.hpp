#ifndef table_hpp
#define table_hpp

#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace Mach {

/******************************************************************************/
struct Table
{
    Table();
    
    void add_entry(const std::string& label);
    void add_string(const std::string& str);

    bool parse(std::ifstream& in);
    void print(std::ostream& out);
    bool write(std::ofstream& out, size_t* end);
                
    struct Entry {
        Entry();
        uint32_t    strx;
        uint8_t     type;
        uint8_t     sect;
        int16_t     desc;
        uint64_t    val;
        bool read(std::ifstream& in);
        void write(std::ofstream& in);
    };
    
    std::vector<Entry> entries;
    std::map<size_t, std::string> strings;
    
    uint32_t write_size();
        
  private:
    bool parse_entries(std::ifstream& in, int num);
    bool parse_strings(std::ifstream& in, int size);
        
    size_t intern(const std::string& label);
};

}

#endif
