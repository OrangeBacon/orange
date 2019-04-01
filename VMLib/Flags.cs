using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace VMLib {
    // all control bits that influence the computer
    public class Flags : Component {
        
        public ObservableCollection<KeyValuePair<string, bool>> FlagsAvaliable { get; } = new ObservableCollection<KeyValuePair<string, bool>>();

        public Flags(VMCore core) : base(core, nameof(Flags)) {
        }

        public int Add(string name, bool value) {
            FlagsAvaliable.Add(new KeyValuePair<string, bool>(name, value));
            OnPropertyChanged("FlagsAvaliable");
            return FlagsAvaliable.Count - 1;
        }

        public void Update(int name, bool value) {
            FlagsAvaliable[name] = new KeyValuePair<string, bool>(FlagsAvaliable[name].Key, value);
            OnPropertyChanged("FlagsAvaliable");
        }
    }
}
