using System;
using System.ComponentModel;

namespace VMLib {
    public class ALU : Component {
        private readonly Bus LeftBus;
        private readonly Bus RightBus;
        private readonly Bus OutBus;
        private Flags Flags;

        private readonly int overflowFlag;
        private readonly int zeroFlag;

        public ALU(VMCore core, Bus leftBus, Bus rightBus, Bus outBus) : base(core, nameof(ALU)) {
            LeftBus = leftBus;
            RightBus = rightBus;
            OutBus = outBus;

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
            Commands.Add(new MicrocodeCommand("ALU Mode 0", ()=>SetMode(0)) {
                Changes = { this }
            });
            Commands.Add(new MicrocodeCommand("ALU Mode 1", ()=>SetMode(1)) {
                Changes = { this }
            });
            Commands.Add(new MicrocodeCommand("ALU Mode 2", ()=>SetMode(2)) {
                Changes = { this }
            });
            Commands.Add(new MicrocodeCommand("ALU Mode 3", ()=>SetMode(3)) {
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
                    var result = unchecked((ushort)(left - right));
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
                case 2: { // negate
                    var result = unchecked((ushort)-left);
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 3: { // bit and
                    var result = unchecked((ushort)(left & right));
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 4: { // bit or
                    var result = unchecked((ushort)(left | right));
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 5: {  // bit xor
                    var result = unchecked((ushort)(left ^ right));
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 6: {  // bit not (1's comp.)
                    var result = unchecked((ushort)~left);
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 7: { // left shift
                    var result = unchecked((ushort)(left << right));
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 8: { // cyclic left shift
                    var result = unchecked((ushort)(left << right | left >> (16 - right)));
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 9: { // logical right shift
                    var result = unchecked((ushort)(left >> right));
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 10: { // arithmetic right shift
                    int l = unchecked((short)left);
                    int r = unchecked((short)right);
                    var result = unchecked((ushort)(left >> right));
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                case 11: { // cyclic right shift
                    var result = unchecked((ushort)(left >> right | (left & ((1 << right) - 1) << right )));
                    OutBus.Write(result);
                    if(result == 0) {
                        Flags.Update(zeroFlag, true);
                    } else {
                        Flags.Update(zeroFlag, false);
                    }
                }; break;
                default: {
                    VMCore.Log.Warn("Invalid ALU Operation");
                }; break;
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
        public void SetMode(int bit) {
            Mode ^= unchecked((byte)(1 << bit));
        }
    }
}
