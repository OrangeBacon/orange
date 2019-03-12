using static VMLib.Util;
using static VMLib.VMCore;

namespace VMLib {
    // linear 64kB memory
    public class Memory64k : Component {
        private readonly ushort[] memory = new ushort[ushort.MaxValue];

        private Bus address;

        public Memory64k(VMCore core, string name, LogicState<Bus> address, params LogicState<Bus>[] outputs)
            : base(core, nameof(Memory64k)) {

            // should address just be a bus? or have to be InputState(Bus)?
            if(address.State != LogicState.In) {
                Log.Warn("Can only use input state for memory address");
            }

            this.address = address.Value;

            // allow for multiple input and output channels in the memory
            // assume only one input or one output is active concurrently
            // this assumption is not in the code,
            foreach(var output in outputs) {
                if((output.State & LogicState.Out) == LogicState.Out) {
                    
                    // read from memory to bus
                    Commands.Add(new MicrocodeCommand($"Memory({address.Value.Name}) -> {output.Value.Name}",
                        Bind(MemoryRead, output.Value)) {
                        Depends = { this, address.Value },
                        Changes = { output.Value }
                    });
                }

                if((output.State & LogicState.In) == LogicState.In) {

                    // write from bus to memory
                    Commands.Add(new MicrocodeCommand($"{output.Value.Name} -> Memory({address.Value.Name})", 
                        Bind(MemoryWrite,output.Value)) {
                        Depends = { address.Value, output.Value },
                        Changes = { this }
                    });
                }
            }

            core.Memory = this;

        }

        internal void MemoryRead(Bus output) {
            output.Write(memory[address.Read()]);
        }

        public ushort MemoryRead(ushort address) {
            return memory[address];
        }

        internal void MemoryWrite(Bus output) {
            memory[address.Read()] = output.Read();
        }

        public void MemoryWrite(ushort address, ushort value) {
            memory[address] = value;
        }
    }
}
