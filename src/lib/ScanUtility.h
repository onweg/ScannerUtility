#pragma once

#ifdef MYLIB_EXPORTS
    #define MYLIB __declspec(dllexport)
#else
    #define MYLIB __declspec(dllimport)
#endif

class MYLIB ScanUtility
{
private:
    int _a, _b;

public:
    ScanUtility(int a, int b);
    ~ScanUtility();
    int StartScan();
};
