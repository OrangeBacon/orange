using Interface.Framework;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using VMLib;

namespace Interface.Views {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MicrocodeProcess : Window {
        public MicrocodeProcess() {
            InitializeComponent();
            ((ViewModels.MicrocodeProcess)DataContext).ShowLogEvent += ShowLog;
        }

        static Window LogWindow;
        private void ShowLog(object sender, EventArgs e) {
            if (LogWindow == null) {
                LogWindow = new Log {
                    Owner = this,
                    DataContext = Globals.Log
                };
                LogWindow.Show();
                LogWindow.Closed += LogClose;
                return;
            }
            if(LogWindow.WindowState == WindowState.Minimized) {
                LogWindow.WindowState = WindowState.Normal;
            }
            LogWindow.Activate();
            
        }

        private void LogClose(object sender, EventArgs e) {
            LogWindow = null;
        }
    }
}
