using System.ComponentModel;

namespace VMLib {
    public class ALU : Component {
        private readonly Bus LeftBus;
        private readonly Bus RightBus;
        private readonly Bus OutBus;
        private Flags Flags;

        private readonly int overflowFlag;
        private readonly int zeroFlag;
        private readonly VMCore core;

        public ALU(VMCore core, Bus leftBus, Bus rightBus, Bus outBus) : base(core, nameof(ALU)) {
            LeftBus = leftBus;
            RightBus = rightBus;
            OutBus = outBus;
            this.core = core;
            Flags = core.Flags;
            core.PropertyChanged += (sender, e) => {
                if(e.PropertyName == "Flags") {
                    Flags = ((VMCore)sender).Flags;
                }
            };

            // should the ALU output to the bus?
            Commands.Add(new MicrocodeCommand("ALU Output", SetWriteEnable) {
                Changes = { outBus, Flags },
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
            overflowFlag = Flags.Add("overflow/carry", false);
            zeroFlag = Flags.Add("Zero", false);
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
                    var result = unchecked((ushort)(left + right));
                    OutBus.Write(result);
                    Flags.Update(overflowFlag, true);
                    if(result != left + right) {
                        Flags.Update(overflowFlag, true);
                    } else {
                        Flags.Update(overflowFlag, false);
                    }
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 1: { // Subtract
                    var result = (ushort)(left - right);
                    OutBus.Write(result);
                    if(result != left - right) {
                        Flags.Update(overflowFlag, true);
                    } else {
                        Flags.Update(overflowFlag, false);
                    }
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
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
