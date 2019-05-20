namespace VMLib {
    internal class InstructionRegister : Component {
        public ushort OpCode { get; private set; }
        public ushort Arg0 { get; private set; }
        public ushort Arg1 { get; private set; }
        public ushort Arg2 { get; private set; }

        private ushort[] aArr = new ushort[3];
        public ushort[] Args {
            get {
                aArr[0] = Arg0;
                aArr[1] = Arg1;
                aArr[2] = Arg2;
                return aArr;
            }
        }

        public InstructionRegister(VMCore core, Bus instBus, Bus dataBus) : base(core, nameof(InstructionRegister)) {
            Commands.Add(new MicrocodeCommand("Instruction Register In", () => {
                OpCode = (ushort)((instBus.Read() >> 9) & 0b1111111); // 7*1, 9*0
                Arg0 = (ushort)((instBus.Read() >> 6) & 0b111); // 7*0, 3*1, 6*0
                Arg1 = (ushort)((instBus.Read() >> 3) & 0b111); // 7*0, 3*0, 3*1, 3*0
                Arg2 = (ushort)((instBus.Read() >> 0) & 0b111); // 7*0, 6*0, 3*1
                OnPropertyChanged("OpCode");
                OnPropertyChanged("Arg0");
                OnPropertyChanged("Arg1");
                OnPropertyChanged("Arg2");
            }) { Changes = { this }, Depends = { instBus } });

            Commands.Add(new MicrocodeCommand("Intermediate1", () => {
                dataBus.Write(Arg0);
            }) { Changes = { dataBus }, Depends = { this } });

            Commands.Add(new MicrocodeCommand("Intermediate2", () => {
                dataBus.Write(Arg1);
            }) { Changes = { dataBus }, Depends = { this } });

            Commands.Add(new MicrocodeCommand("Intermediate3", () => {
                dataBus.Write(Arg2);
            }) { Changes = { dataBus }, Depends = { this } });

            Commands.Add(new MicrocodeCommand("Long Intermediate", () => {
                dataBus.Write((ushort)(Arg1 << 3 | Arg2));
            }) { Changes = { dataBus }, Depends = { this } });
        }
    }
}