#include "shared/platform.h"
#include "emulator/emu.h"
#include "emulator/emulator.h"
#include "emulator/vmcore.h"
#include "emulator/register.h"

void emulator() {
    cOutPrintf(TextGreen, "Emulation\n");

    VMCore core;
    vmcoreInit(&core);

    unsigned int accumulator = createRegister(&core);
    unsigned int R1 = createRegister(&core);
    unsigned int dataBus = createBus(&core);
    
    unsigned int dataToAcc = regConnectBus(&core, accumulator, dataBus);
    unsigned int dataToR1  = regConnectBus(&core, R1, dataBus);

    regWriteInt(&core, accumulator, 15);
    coreCall(&core, dataToAcc + 1);
    coreCall(&core, dataToR1);

    cOutPrintf(TextWhite, "%i\n", regRead(&core, R1));
}