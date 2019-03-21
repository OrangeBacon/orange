using System.Collections.Generic;

namespace VMLib {
    // all control bits that influence the computer
    public class Flags : Component {
        public Dictionary<string, bool> FlagsAvaliable { get; } = new Dictionary<string, bool>();

        public Flags(VMCore core) : base(core, nameof(VMLib.Flags)) {

        }

        public void Add(string name, bool value) {
            FlagsAvaliable.Add(name, value);
        }

        public void Update(string name, bool value) {
            FlagsAvaliable[name] = value;
            OnPropertyChanged("FlagsAvaliable");
        }
    }
}
