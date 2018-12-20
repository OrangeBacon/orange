using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Interface.Framework;
using Interface;
using System.Windows;

namespace Interface.ViewModels {
    class MicrocodeProcess : ObservableObject {
        public DelagateCommand ShowLog { get; } = new DelagateCommand(() => {
            var win = new Views.Log();
            win.DataContext = (Models.App)Application.Current.Properties[Models.App.Name];
            win.Show();
        });
    }
}
