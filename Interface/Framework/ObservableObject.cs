using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;
using System.Diagnostics;

namespace Interface.Framework {
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
                Debug.Fail($"Property '{name}' not found on class {this.GetType().Name}");
            }
        }
    }
}
