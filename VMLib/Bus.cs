namespace VMLib {
    public class Bus : Component {
        // value currently being output onto the bus
        public short Value { get; private set; }

        // TODO: when component no longer outputing, the bus will be
        // disconnected, so electricaly will have an undefined value,
        // currently this class stores previous value indefinately.
        public Bus(VMCore core, string name) : base(core, name + " " + nameof(Bus)) { }

        public short Read() {
            return Value;
        }

        public void Write(short val) {
            Value = val;
            OnPropertyChanged("Value");
        }
    }
}
