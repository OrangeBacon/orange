using Interface.Framework;
using System;
using System.Windows;


namespace Interface.Views {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MicrocodeProcess : Window {
        public MicrocodeProcess() {
            InitializeComponent();
            ((ViewModels.MicrocodeProcess)DataContext).ShowLogEvent += ShowLog;
        }

        // cache of the log window's main object
        static Window LogWindow;

        // show the log to the user, run on the ShowLog event
        private void ShowLog(object sender, EventArgs e) {
            //  if the log is closed, open a new window
            if (LogWindow == null) {
                LogWindow = new Log {
                    DataContext = Globals.Log
                };
                LogWindow.Show();
                LogWindow.Closed += LogClose;
                return;
            }
            // else it is possibly minimised and/or not in focus
            if(LogWindow.WindowState == WindowState.Minimized) {
                // un minimise it (does not focus the window)
                LogWindow.WindowState = WindowState.Normal;
            }
            // focus the window
            LogWindow.Activate();
            
        }

        // remove log from cache as the window is closed
        private void LogClose(object sender, EventArgs e) {
            LogWindow = null;
        }

        // run when the main window's close button is clicked
        private void WindowClosed(object sender, EventArgs e) {
            // closes all windows in the application
            // without this, the log would remain open, with the main window closed.
            Application.Current.Shutdown();
        }
    }
}
