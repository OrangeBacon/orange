using System;
using System.Collections.Generic;
using static VMLib.Util;

namespace VMLib {
    public class StarfishVM { 
        public static VMCore CreateStarfishVM(ILogger Log = null) {
            // create default virtual machine
            VMCore VM = new VMCore();

            // should the global log be set up?
            if(Log != null) {
                VMCore.Log = Log;
            }

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
            addrBus.Write(1);

            var ALU = new ALU(VM, leftBus, rightBus, dataBus);

            var regA = new Register(VM, "A", InOutState(dataBus), OutState(leftBus));
            var regB = new Register(VM, "B", InOutState(dataBus), OutState(leftBus));
            var regC = new Register(VM, "C", InOutState(dataBus), OutState(rightBus));
            var regD = new Register(VM, "D", InOutState(dataBus), OutState(rightBus));
            var regE = new Register(VM, "E", InOutState(dataBus), OutState(rightBus));

            var regAR = new Register(VM, "AR", InOutState(dataBus), OutState(addrBus));
            var regSP = new Register(VM, "SP", InOutState(dataBus), OutState(addrBus));
            var regIP = new Register(VM, "IP", InOutState(dataBus), OutState(addrBus));

            var mem = new Memory64k(VM, "Main Memory", InState(addrBus), InOutState(dataBus), OutState(instBus));

            var IR = new InstructionRegister(VM, instBus);
            VM.Controller.Input(IR, "OpCode", 7);

            return VM;
        }
    }
}
