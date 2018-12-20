using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VMLib;

namespace Interface {
    class StarfishVM {
        public readonly VMCore VM;

        public StarfishVM(ILogger logger = null) {
            VM = new VMCore();
            if(logger != null) {
                VM.Log = logger;
            }

            var dataBus = new Bus(VM);
            var regA = new Register(VM, "A", dataBus);
            var regB = new Register(VM, "B", dataBus);
        }
    }
}
