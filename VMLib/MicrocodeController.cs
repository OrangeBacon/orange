using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class MicrocodeController {
        public readonly List<MicrocodeCommand> Commands;

        public void AddRange(List<MicrocodeCommand> commands) {
            Commands.AddRange(commands);
        }
    }
}
