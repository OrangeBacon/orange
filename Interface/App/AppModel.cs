using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Interface.Models {
    class App : Framework.ObservableObject {
        public static string TypeName = nameof(App);

        public ObservableCollection<string> Log { get; } = new ObservableCollection<string>();
    }
}
