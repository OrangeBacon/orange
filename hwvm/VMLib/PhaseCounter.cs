namespace VMLib {
    // phase counter, will be used as input to microcodecontroller
    public class PhaseCounter : Component {
        public ushort Phase { get; private set; } = 0;

        public PhaseCounter(VMCore core) : base(core, "Phase Counter") {
            Commands.Add(new MicrocodeCommand("Reset Phase Counter", ()=> {
                Phase = 0;
                OnPropertyChanged("Phase");
            }));

            // run at start of clock sequence
            core.Clock.At(0).Add(() => {
                // if reset, the counter should not be incremented
                Phase += 1;
                OnPropertyChanged("Phase");
            });
        }
    }
}
