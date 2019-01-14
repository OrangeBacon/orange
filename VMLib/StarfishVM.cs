using System;
using System.Collections.Generic;
using static VMLib.Util;

namespace VMLib {
    public class StarfishVM { 
        public static VMCore CreateStarfishVM() {
            // create default virtual machine
            var VM = new VMCore();
            var phase = new PhaseCounter(VM);

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

            return VM;
        }
    }
}
