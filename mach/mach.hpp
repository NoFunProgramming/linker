#ifndef mach_hpp
#define mach_hpp

#include "segment.hpp"
#include "table.hpp"

namespace Mach {

/******************************************************************************/
#define CPU_Arch_Mask       0xff000000
#define CPU_Arch_ABI64      0x01000000

#define CPU_Type_MC680x0    6
#define CPU_Type_X86        7
#define CPU_Type_I386       CPU_Type_X86
#define CPU_Type_X86_64     CPU_Type_X86 | CPU_Arch_ABI64

#define CPU_Subtype_X86_All      3
#define CPU_Subtype_X86_64_All   3

/******************************************************************************/
class Mach
{
  public:
    Mach();
    bool parse(std::ifstream& in);
    void print(std::ostream& out);
    void write(std::ofstream& out);

    std::vector<std::unique_ptr<Segment>> segments;
    std::vector<std::unique_ptr<Table>> tables;
    
    uint32_t magic;
    uint32_t cpu_type;
    uint32_t cpu_subtype;
    uint32_t file_type;
    uint32_t flags;
};

}

#endif
