#include <fstream>

#include "utility.hpp"
#include "Assembler.hpp"

namespace dcpu11
{
    char u4ToHexChar(u8 n)
    {
        n &= 0xf;
        if(n < 10)
            return '0' + n;
        return 'a' + (n - 10);
    }

    void u16ToHexStr(u16 n, char str[4])
    {
        u16 m = 0x8000; // bit mask 1000 0000 0000 0000
        u8 d = 0;
        u8 di = 0;

        for(u8 i = 0; i < 16; i++)
        {
            if((n & m) != 0)
                d |= 1;

            if((i+1) % 4 == 0)
            {
                str[di] = u4ToHexChar(d);
                d = 0;
                di++;
            }

            m >>= 1;
            d <<= 1;
        }
    }

    bool loadProgram(DCPU & cpu, const std::string & filename)
    {
        std::ifstream ifs(filename.c_str(), std::ios::binary|std::ios::in);
        if(!ifs.good())
        {
            ifs.close();
            std::cout << "Error: cannot open file '" << filename << "'" << std::endl;
            return false;
        }

        Assembler assembler;
        std::cout << "Start assembling..." << std::endl;
        bool res = assembler.assembleStream(ifs);
        if(!res)
            std::cout << "Error: " << assembler.getExceptionString() << std::endl;
        else
            cpu.setMemory(assembler.getAssembly());
        std::cout << "Assembling finished." << std::endl;

        ifs.close();
        return res;
    }

    bool dumpAsText(DCPU & cpu, const std::string & filename)
    {
        std::ofstream ofs(filename.c_str(), std::ios::binary|std::ios::out|std::ios::trunc);
        if(!ofs.good())
        {
            ofs.close();
            std::cout << "Error: cannot create file '" << filename << "'" << std::endl;
            return false;
        }

        std::cout << "Dumping CPU memory..." << std::endl;

        char str[4];
        for(u32 i = 0; i < 65536; i++)
        {
            if(i % 8 == 0)
            {
                u16ToHexStr(i, str);
                if(i != 0)
                    ofs << std::endl;
                ofs << str << ": ";
            }
            u16ToHexStr(cpu.getMemory(i), str);
            ofs << str << " ";
        }

        std::cout << "Dumping finished." << std::endl;

        ofs.close();
        return true;
    }

} // namespace dcpu11


