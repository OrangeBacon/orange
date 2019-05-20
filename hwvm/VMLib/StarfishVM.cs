using System;
using System.Collections.Generic;
using static VMLib.Util;

namespace VMLib {
    public class StarfishVM { 
        public static VMCore CreateStarfishVM(ILogger Log = null) {
        
            // should the global log be set up?
            if (Log != null) {
                VMCore.Log = Log;
            }

            // create default virtual machine
            VMCore VM = new VMCore();

            var phase = new PhaseCounter(VM);
            VM.Controller.Input(phase, "Phase", 4);

            // NOTE: The order that these objects are constructed
            // equals the order that the microcode commands are
            // registered - therefore if rearranged, other components
            // might break in the furure (microcode assembler)

            var dataBus = new Bus(VM, "Data");
            var leftBus = new Bus(VM, "ALU Left");
            var rightBus = new Bus(VM, "ALU Right");
            var addrBus = new Bus(VM, "Address");
            var instBus = new Bus(VM, "Instruction");

            var IR = new InstructionRegister(VM, instBus, dataBus);
            VM.Controller.Input(IR, "OpCode", 7);

            new ALU(VM, leftBus, rightBus, dataBus);
            new RegisterController(VM, 8, IR,
                InOutState(dataBus), OutState(leftBus), OutState(rightBus), OutState(addrBus));
            new Register(VM, "Temp", InOutState(dataBus), OutState(leftBus), OutState(rightBus), OutState(addrBus));

            new Memory64k(VM, "Main Memory", InState(addrBus), InOutState(dataBus), OutState(instBus));

            return VM;
        }
    }
}
