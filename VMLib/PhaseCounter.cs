using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class PhaseCounter : Component {
        public short Phase { get; private set; } = 0;

        private bool reset = false;
        public PhaseCounter(VMCore core) : base(core, "Phase Counter") {
            Commands.Add(new MicrocodeCommand("Reset Phase Counter", ()=> {
                Phase = 0;
                reset = true;
            }));
            core.Clock.At(0).Add(() => {
                if(!reset) {
                    Phase += 1;
                }
                reset = false;
                OnPropertyChanged("Phase");
            });
        }
    }
}
