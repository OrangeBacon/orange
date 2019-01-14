using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    internal class InstructionRegister : Component {
        public ushort OpCode { get; private set; }
        public ushort Arg1 { get; private set; }
        public ushort Arg2 { get; private set; }
        public ushort Arg3 { get; private set; }

        public InstructionRegister(VMCore core, Bus instBus) : base(core, nameof(InstructionRegister)) {
            Commands.Add(new MicrocodeCommand("Instruction Register In", () => {
                OpCode = (ushort)((instBus.Read() >> 9) & 0b1111111); // 7*1, 9*0
                Arg1   = (ushort)((instBus.Read() >> 6) & 0b111); // 7*0, 3*1, 6*0
                Arg2   = (ushort)((instBus.Read() >> 3) & 0b111); // 7*0, 3*0, 3*1, 3*0
                Arg3   = (ushort)((instBus.Read() >> 0) & 0b111); // 7*0, 6*0, 3*1
                OnPropertyChanged("OpCode");
                OnPropertyChanged("Arg1");
                OnPropertyChanged("Arg2");
                OnPropertyChanged("Arg3");
            }));
        }
    }
}