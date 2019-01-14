using static VMLib.Util;
using static VMLib.VMCore;

namespace VMLib {
    // linear 64kB memory
    public class Memory64k : Component {
        // 65536 = 2^16
        private readonly ushort[] memory = new ushort[65536];

        public Memory64k(VMCore core, string name, LogicState<Bus> address, params LogicState<Bus>[] outputs)
            : base(core, nameof(Memory64k)) {
            memory[1] = 0b0101010101010101;
                        memory[1] = 0b0101010101010101;

            if(address.State != LogicState.In) {
                Log.Warn("Can only use in state for memory address");
            }
            foreach(var output in outputs) {
                if((output.State & LogicState.Out) == LogicState.Out) {
                    Commands.Add(new MicrocodeCommand($"Memory({address.Value.Name}) -> {output.Value.Name}", () => {
                        output.Value.Write(memory[address.Value.Read()]);
                    }) { Depends = { this, address.Value }, Changes = { output.Value } });
                }

                if((output.State & LogicState.In) == LogicState.In) {
                    Commands.Add(new MicrocodeCommand($"{output.Value.Name} -> Memory({address.Value.Name})", () => {
                        memory[address.Value.Read()] = output.Value.Read();
                    }) { Depends = { address.Value, output.Value }, Changes = { this } });
                }
            }
        }
    }
}
