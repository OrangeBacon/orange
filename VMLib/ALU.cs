using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
            Flags = flags;
            Commands.Add(new MicrocodeCommand("ALU Output", SetWriteEnable, this) {
                Changes = { outBus, flags },
                Depends = { leftBus, rightBus, this }
            });
            Commands.Add(new MicrocodeCommand("ALU Mode", SetMode1, this) {
                Changes = { this }
            });

            LeftBus.PropertyChanged += Update;
            RightBus.PropertyChanged += Update;
        }

        public override void OnClockTick() {
            if(WriteEnable) {
                Update(null, new PropertyChangedEventArgs("Value"));
                WriteEnable = false;
            }
        }

        private void Update(object sender, PropertyChangedEventArgs e) {
            if(e.PropertyName != "Value" || WriteEnable == false)
                return;
            var left = LeftBus.Read();
            var right = RightBus.Read();
            switch(Mode) {
                case 0: {
                    OutBus.Write((short)(left + right));
                    if(left + right != (short)(left + right)) {
                        Flags.Store(1);
                    }
                }; break;
                case 1: {
                    OutBus.Write((short)(left - right));
                }; break;
                default: break;
            }
        }

        public byte Mode { get; private set; } = 0;
        private bool WriteEnable = false;
        public void SetWriteEnable() {
            WriteEnable = true;
            Mode = 0;
        }
        public void SetMode1() {
            Mode ^= 1 << 0;
        }
        public void SetMode2() {
            Mode ^= 1 << 1;
        }
        public void SetMode3() {
            Mode ^= 1 << 2;
        }
    }
}
