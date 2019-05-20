using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Diagnostics;

namespace VMLib {

    // simple inotifypropertychanged implementation for components 
    // and interface code to use
    public abstract class ObservableObject : INotifyPropertyChanged {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string prop = "") {
            VerifyPropertyName(prop);

            if (prop == "") {
                throw new ArgumentOutOfRangeException("prop", "No property gathered");
            }

            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(prop));
        }

        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        protected void VerifyPropertyName(string name) {
            if(TypeDescriptor.GetProperties(this)[name] == null) {
                Debug.Fail($"Property '{name}' not found on class {GetType().Name}");
            }
        }
    }
}
