using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;

namespace Interface.Framework {
    class ObservableObject : INotifyPropertyChanged {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void RaisePropertyChanged([CallerMemberName] string prop = "") {
            if (prop == "") {
                throw new ArgumentOutOfRangeException("prop", "No property gathered");
            }

            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(prop));
        }
    }
}
