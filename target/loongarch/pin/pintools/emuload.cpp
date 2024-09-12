// 记录数据从内存加载到寄存器的过程

#include <stdio.h>
#include "pintool.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "../reg.h"
#include "../string_convert.h"
#include "../ins_inspection.h"

using std::cerr;
using std::endl;
using std::ios;
using std::setw;
using std::hex;

// 修改输出文件路径
std::string OutFilePath = "/home/myb/pintool_out/mem_access/emuload.txt";
std::ofstream out;


static VOID EmitMem(VOID* ea, INT32 size)
{
    switch (size)
    {
        case 0:
            out << setw(1);
            break;

        case 1:
            out << static_cast< UINT32 >(*static_cast< UINT8* >(ea));
            break;

        case 2:
            out << *static_cast< UINT16* >(ea);
            break;

        case 4:
            out << *static_cast< UINT32* >(ea);
            break;

        case 8:
            out << *static_cast< UINT64* >(ea);
            break;

        default:
            out.unsetf(ios::showbase);
            out << setw(1) << "0x";
            for (INT32 i = 0; i < size; i++)
            {
                out << static_cast< UINT32 >(static_cast< UINT8* >(ea)[i]);
            }
            out.setf(ios::showbase);
            break;
    }
}

BOOL IsMemToReg(INS ins)
{
    switch (INS_Opcode(ins))
    {
    case LISA_LD_B:
    case LISA_LD_H:
    case LISA_LD_W:
    case LISA_LD_D:
    case LISA_LD_BU:
    case LISA_LD_HU:
    case LISA_LD_WU:
        return true;
    default:
        return false;
    }
}

UINT32 RegWriteIndex(INS ins) {
    UINT32 destination_registers = 0;
    for (int i = 0; i < ins->origin_ins->opnd_count; ++i) {
        LISA_REG_ACCESS_TYPE access_type = get_reg_access_type(ins->origin_ins, i);
        if (access_type == GPR_WRITE || access_type == GPR_READWRITE) {
            destination_registers = ins->origin_ins->opnd[i].val;
            break;
        }
    }
    int reg_index = destination_registers & 0xff;
    if (reg_index < 32)
        return gpr_to_REG(reg_index);
    else
        return fpr_to_REG(reg_index - 32);
}


// Move from memory to register
VOID DoLoad(REG reg, VOID* addr, INT32 size)
{
    out << "Memory addr: " << addr << " Reg: " << REG_StringShort(reg) << " Data: " << hex;
    EmitMem(addr, size);
    out << endl;
}

VOID EmulateLoad(INS ins, VOID* v)
{
    if (IsMemToReg(ins))
    {
        UINT32 write_reg_index = RegWriteIndex(ins);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoLoad),
                        IARG_UINT32, write_reg_index,
                        IARG_MEMORYREAD_EA,
                        IARG_MEMORYREAD_SIZE, IARG_END);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool emulates loads" << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    out.open(OutFilePath);
    if (PIN_Init(argc, argv)) return Usage();
    INS_AddInstrumentFunction(EmulateLoad, 0);
    // PIN_StartProgram(); // Never returns
    return 0;
}
