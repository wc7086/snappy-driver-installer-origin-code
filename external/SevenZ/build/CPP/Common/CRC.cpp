// Common/CRC.cpp

#include "StdAfx.h"

#include "../../C/7zCrc.h"

__declspec(dllexport) struct CCRCTableInit { CCRCTableInit() { CrcGenerateTable(); } } g_CRCTableInit;

void registercrc()
{
	CCRCTableInit *a;
}
