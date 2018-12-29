using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VMLib;
using static VMLib.Util;

namespace VMLib {
    public class StarfishVM { 
        public static VMCore CreateStarfishVM() {
            var VM = new VMCore();

            var dataBus = new Bus(VM, "Data");
            var leftBus = new Bus(VM, "ALU Left");
            var rightBus = new Bus(VM, "ALU Right");

            var flags = new Register(VM, "flags");
            var ALU = new ALU(VM, leftBus, rightBus, dataBus, flags);
            leftBus.Write(5);
            rightBus.Write(7);

            var regA = new Register(VM, "A", ThreeState(dataBus), OutState(leftBus));
            var regB = new Register(VM, "B", ThreeState(dataBus), OutState(rightBus));

            var phase = new PhaseCounter(VM);

            return VM;
        }
    }
}
