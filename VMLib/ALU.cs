using System.ComponentModel;

namespace VMLib {
    public class ALU : Component {
        private readonly Bus LeftBus;
        private readonly Bus RightBus;
        private readonly Bus OutBus;
        private readonly Register Flags;

        public ALU(VMCore core, Bus leftBus, Bus rightBus, Bus outBus, Register flags) : base(core, nameof(ALU)) {
            LeftBus = leftBus;
            RightBus = rightBus;
            OutBus = outBus;

            // no flags are currently written to this register
            // not used yet but will be in the future.
            Flags = flags;

            // should the ALU output to the bus?
            Commands.Add(new MicrocodeCommand("ALU Output", SetWriteEnable) {
                Changes = { outBus, flags },
                Depends = { leftBus, rightBus, this }
            });

            // which computation should be performed?
            Commands.Add(new MicrocodeCommand("ALU Mode", SetMode1) {
                Changes = { this }
            });

            // ensure output value is updated if inputs are changed
            LeftBus.PropertyChanged += Update;
            RightBus.PropertyChanged += Update;

            core.Clock.AtEnd().Add(() => {
                // stop updating output to the bus when the clock cycle ends
                // (microcode nolonger asserting alu output signal)
                WriteEnable = false;

                // mode change signals no longer asserted
                Mode = 0;
            });
        }

        private void Update(object sender, PropertyChangedEventArgs e) {
            // irrelevant property changed
            // or no output required
            if(e.PropertyName != "Value" || WriteEnable == false)
                return;

            // data
            var left = LeftBus.Read();
            var right = RightBus.Read();

            switch(Mode) {
                case 0: { // Add
                    OutBus.Write((short)(left + right));
                }; break;
                case 1: { // Subtract
                    OutBus.Write((short)(left - right));
                }; break;
                default: break;
            }
        }

        // which calculation?
        public byte Mode { get; private set; } = 0;

        // should result of calaculation be output to the bus?
        private bool WriteEnable = false;

        // enable output, output initial value
        public void SetWriteEnable() {
            WriteEnable = true;
            Update(null, new PropertyChangedEventArgs("Value"));
        }

        // set bit 0 of mode
        public void SetMode1() {
            Mode ^= 1 << 0;
        }
    }
}
