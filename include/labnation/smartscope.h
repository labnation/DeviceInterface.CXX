#ifndef _LABNATION_SMARTSCOPE_H
#define _LABNATION_SMARTSCOPE_H

#include "hardware.h"
namespace labnation {

class SmartScope : public Hardware {
public:
    enum Controller
    {
        PIC = 0,
        ROM = 1,
        FLASH = 2,
        FPGA = 3,
        AWG = 4
    };
    virtual void GetControllerRegister(Controller ctrl, uint address, uint length, void* data);
    virtual void SetControllerRegister(Controller ctrl, uint address, void* data);
    virtual int GetAcquisition(void* buffer);
    virtual void* GetData(int length);
    virtual void FlushDataPipe();
    virtual void Reset();
    virtual bool FlashFpga(void* firmware);
    virtual void* GetPicFirmwareVersion();
    virtual bool IsDestroyed();
    virtual void Destroy();
};
}

#endif //_LABNATION_SMARTSCOPE_H