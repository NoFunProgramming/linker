#ifndef segment_hpp
#define segment_hpp

#include "section.hpp"

namespace Mach {

/******************************************************************************/
class Segment
{
  public:
    Segment();
    
    bool parse(std::ifstream& in);
    void print(std::ostream& out);
    bool write(std::ofstream& out, size_t* end);
        
    std::string name;
    
    uint32_t protect_max;
    uint32_t protect_init;
    uint32_t flags;
    uint64_t addr;
    
    std::vector<std::unique_ptr<Section>> sections;
    
    uint32_t cmd_size();
    
  private:
    std::unique_ptr<Section> make_section(std::ifstream& in);
};

}

#endif
