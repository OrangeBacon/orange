using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;

namespace Interface.ViewModels {
    class App : Framework.ObservableObject {
        public readonly ObservableCollection<string> Log = new ObservableCollection<string>();
    }
}
