using System.Collections.Generic;

namespace VMLib {
    // all control bits that influence the computer
    public class Flags : Component {
        private readonly Dictionary<string, bool> flags = new Dictionary<string, bool>();

        public ushort Value {
            get {
                ushort ret = 0;
                int i = 0;
                foreach(var item in flags) {
                    ret = (ushort)(ret | (item.Value?1:0) << i);
                    i++;
                }
                return ret;
            }
        }

        public Flags(VMCore core) : base(core, nameof(Flags)) {

        }

        public void Add(string name, bool value) {
            flags.Add(name, value);
        }

        public void Update(string name, bool value) {
            flags[name] = value;
            OnPropertyChanged("Value");
        }
    }
}
