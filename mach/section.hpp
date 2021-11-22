#ifndef section_hpp
#define section_hpp

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

namespace Mach
{

/******************************************************************************/
struct Section
{
  public:
    Section();
    
    bool parse(std::ifstream& in);
    void print(std::ostream& out);
    bool write(std::ofstream& out, size_t* end);
                
    std::string name;
    std::string segment;
    
    uint64_t addr;
    uint32_t align;
    uint32_t flags;
        
    uint32_t cmd_size();
    
    std::vector<uint8_t> data;
    
  protected:
    virtual bool parse_data(std::ifstream& in, size_t size);
    virtual bool print_data(std::ostream& out);
    virtual bool write_data(std::ofstream& out);
};

}

#endif
