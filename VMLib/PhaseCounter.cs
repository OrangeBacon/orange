namespace VMLib {
    // phase counter, will be used as input to microcodecontroller
    public class PhaseCounter : Component {
        public short Phase { get; private set; } = 0;

        private bool reset = false;
        public PhaseCounter(VMCore core) : base(core, "Phase Counter") {
            Commands.Add(new MicrocodeCommand("Reset Phase Counter", ()=> {
                Phase = 0;
                reset = true;
            }));

            // run at start of clock sequence
            core.Clock.At(0).Add(() => {
                // if reset, the counter should not be incremented
                if(!reset) {
                    Phase += 1;
                }
                reset = false;
                OnPropertyChanged("Phase");
            });
        }
    }
}
