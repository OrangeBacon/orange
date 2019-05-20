using System.Collections.Generic;
using static VMLib.Util;

namespace VMLib {
    public class RegisterController : Component {
        internal RegisterController(VMCore core, int RegCount, InstructionRegister IR, params LogicState<Bus>[] busses)
            : base(core, nameof(RegisterController)) {

            List<ControlledRegister> registers = new List<ControlledRegister>();

            for (int i = 0; i < RegCount; i++) {
                registers.Add(new ControlledRegister(core, $"{i}"));
            }

            for (int i = 0; i < IR.Args.Length; i++) {
                foreach (var bus in busses) {
                    var iCopy = i;
                    if ((bus.State & LogicState.Out) == LogicState.Out) {
                        Commands.Add(new MicrocodeCommand($"Arg{i} {bus.Value.Name} Out", () => {
                            registers[IR.Args[iCopy]].Out(bus.Value);
                        }));
                    }

                    if ((bus.State & LogicState.In) == LogicState.In) {
                        Commands.Add(new MicrocodeCommand($"Arg{i} {bus.Value.Name} In", () => {
                            registers[IR.Args[iCopy]].In(bus.Value);
                        }));
                    }
                }
            }
        }
    }
}
