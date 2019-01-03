using static VMLib.Util;
using static VMLib.VMCore;

namespace VMLib {
    // linear 64kB memory
    internal class Memory64k : Component {
        // 65536 = 2^16
        private readonly ushort[] memory = new ushort[65536];

        public Memory64k(VMCore core, string name, LogicState<Bus> address, LogicState<Bus> output)
            : base(core, nameof(Memory64k)) {
            if(address.State != LogicState.In) {
                Log.Warn("Can only use in state for memory address");
            }
            if(output.State != (LogicState.Out | LogicState.In)) {
                Log.Warn("Can only use inout state for memory data");
            }
            Commands.Add(new MicrocodeCommand($"Memory({address.Value.Name}) -> {output.Value.Name}", () => {
                output.Value.Write(memory[address.Value.Read()]);
            }) { Depends = { this, address.Value }, Changes = { output.Value } });
            Commands.Add(new MicrocodeCommand($"{output.Value.Name} -> Memory({address.Value.Name})", () => {
                memory[address.Value.Read()] = output.Value.Read();
            }) { Depends = { address.Value, output.Value }, Changes = { this } });
        }
    }
}
