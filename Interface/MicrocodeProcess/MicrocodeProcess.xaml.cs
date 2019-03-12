using Interface.Framework;
using System;
using System.Windows;


namespace Interface.Views {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MicrocodeProcess : Window {

        private SubWindow<Log> Log = new SubWindow<Log>();
        private SubWindow<MemoryViewer> MemoryViewer = new SubWindow<MemoryViewer>();

        public MicrocodeProcess() {
            InitializeComponent();
            ((ViewModels.MicrocodeProcess)DataContext).ShowLogEvent += ShowLog;
            ((ViewModels.MicrocodeProcess)DataContext).ShowMemoryViewerEvent += ShowMemoryViewer;

            Log.DataContext(Globals.Log);
        }

        private void ShowLog(object sender, EventArgs e) {
            Log.Open();
        }
        private void ShowMemoryViewer(object sender, EventArgs e) {
            MemoryViewer.Open();
        }

        // run when the main window's close button is clicked
        private void WindowClosed(object sender, EventArgs e) {
            // closes all windows in the application
            // without this, the log would remain open, with the main window closed.
            Application.Current.Shutdown();
        }
    }
}
