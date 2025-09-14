#include "ScanUtility.h"

ScanUtility::ScanUtility(int a, int b)
    :_a(a), _b(b)
{
}

ScanUtility::~ScanUtility()
{
}

int ScanUtility::StartScan() {
    return _a + _b;
}