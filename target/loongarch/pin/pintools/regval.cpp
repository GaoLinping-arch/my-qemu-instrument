/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool demonstrates the use of the PIN_GetContextRegval API for various types of registers.
// It is used with the regval_app application.

#include <fstream>
#include <cassert>
#include "pintool.h"
#include "../reg.h"
#include "../symbols.h"
#include "../string_convert.h"
#include "../ins_inspection.h"
using std::endl;
using std::hex;
using std::ofstream;
using std::string;

/////////////////////
// GLOBAL VARIABLES
/////////////////////
// 修改输出文件路径
ofstream OutFile;
string OutFilePath = "/home/myb/pintool_out/reg_mix/regval.txt";
// We don't want to print the registers too many times, so we put placeholders in the application to tell the tool
// when to start and stop printing.
volatile bool printRegsNow = false;

#ifdef TARGET_MAC
const char *startRtnName = "_Start";
const char *stopRtnName = "_Stop";
#else
const char *startRtnName = "Start";
const char *stopRtnName = "Stop";
#endif

/////////////////////
// ANALYSIS FUNCTIONS
/////////////////////

// Once this is called, the registers will be printed until EndRoutine is called.
static void StartRoutine() { printRegsNow = true; }

// After this is called, the registers will no longer be printed.
static void StopRoutine() { printRegsNow = false; }

static void PrintRegisters(const CONTEXT* ctxt)
{
    if (!printRegsNow)
        return;

    for (int reg = (int)REG_GPR_BASE; reg <= (int)REG_GPR_LAST; ++reg){
        ADDRINT regval = PIN_GetContextReg(ctxt, (REG)reg);
        OutFile << REG_StringShort((REG)reg) << ": 0x" << hex << regval << endl;
    }
}

/////////////////////
// INSTRUMENTATION FUNCTIONS
/////////////////////

static VOID ImageLoad(IMG img, VOID *v)
{
    RTN StartRtn = RTN_FindByName(img, startRtnName);
    if (RTN_Valid(StartRtn))
    {
        RTN_Open(StartRtn);
        RTN_InsertCall(StartRtn, IPOINT_BEFORE, (AFUNPTR)StartRoutine, IARG_END);
        RTN_Close(StartRtn);
    }

    RTN StopRtn = RTN_FindByName(img, stopRtnName);
    if (RTN_Valid(StopRtn))
    {
        RTN_Open(StopRtn);
        RTN_InsertCall(StopRtn, IPOINT_AFTER, (AFUNPTR)StopRoutine, IARG_END);
        RTN_Close(StopRtn);
    }
}

static VOID Trace(TRACE trace, VOID *v)
{
    // TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)PrintRegisters, IARG_END);
    BBL bbl1 = TRACE_BblHead(trace);
    BBL_InsertCall(bbl1, IPOINT_BEFORE, (AFUNPTR)PrintRegisters, IARG_CONST_CONTEXT, IARG_END);
}



static VOID Fini(INT32 code, VOID *v) {
    OutFile.close();
}

/////////////////////
// MAIN FUNCTION
/////////////////////

int main(int argc, char *argv[])
{
    // Initialize Pin
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    OutFile.open(OutFilePath);
    // Add instrumentation
    IMG_AddInstrumentFunction(ImageLoad, 0);
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    return 0;
}
