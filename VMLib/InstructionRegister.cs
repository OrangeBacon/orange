namespace VMLib {
    internal class InstructionRegister : Component {
        public ushort OpCode { get; private set; }
        public ushort Arg1 { get; private set; }
        public ushort Arg2 { get; private set; }
        public ushort Arg3 { get; private set; }

        public InstructionRegister(VMCore core, Bus instBus, Bus dataBus) : base(core, nameof(InstructionRegister)) {
            Commands.Add(new MicrocodeCommand("Instruction Register In", () => {
                OpCode = (ushort)((instBus.Read() >> 9) & 0b1111111); // 7*1, 9*0
                Arg1 = (ushort)((instBus.Read() >> 6) & 0b111); // 7*0, 3*1, 6*0
                Arg2 = (ushort)((instBus.Read() >> 3) & 0b111); // 7*0, 3*0, 3*1, 3*0
                Arg3 = (ushort)((instBus.Read() >> 0) & 0b111); // 7*0, 6*0, 3*1
                OnPropertyChanged("OpCode");
                OnPropertyChanged("Arg1");
                OnPropertyChanged("Arg2");
                OnPropertyChanged("Arg3");
            }) { Changes = { this }, Depends = { instBus } });

            Commands.Add(new MicrocodeCommand("Intermediate", () => {
                dataBus.Write(Arg3);
            }) { Changes = { dataBus }, Depends = { this } });

            Commands.Add(new MicrocodeCommand("Long Intermediate", () => {
                dataBus.Write((ushort)(Arg2 << 3 | Arg3));
            }) { Changes = { dataBus }, Depends = { this } });
        }
    }
}