using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Interface.Models {
    class App {
        public static string Name = nameof(App);

        public ObservableCollection<string> Log { get; } = new ObservableCollection<string>();
    }
}
