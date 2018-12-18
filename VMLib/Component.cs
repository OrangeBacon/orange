using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace VMLib {
    public class Component : INotifyPropertyChanged {
        static int ComponentIDCounter = 0;
        public readonly int ComponentID;

        public Component(VMCore core) {
            core.Add(this);
            ComponentID = ComponentIDCounter++;
        }

        public void OnPropertyChanged(string prop) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(prop));
        }
        public event PropertyChangedEventHandler PropertyChanged;
    }
}
