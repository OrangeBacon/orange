using System;
using System.Windows;

namespace Interface.Framework {
    internal class SubWindow<T> where T : Window, new() {

        // cache of the log window's main object
        private Window CurrentWindow;

        private object Context;

        // show the log to the user, run on the ShowLog event
        public void Open() { Open(null, null); }
        public void Open(object sender, EventArgs e) {
            //  if the log is closed, open a new window
            if(CurrentWindow == null) {
                CurrentWindow = new T();
                if(Context != null) {
                    CurrentWindow.DataContext = Context;
                }
                CurrentWindow.Show();
                CurrentWindow.Closed += Close;
                return;
            }
            // else it is possibly minimised and/or not in focus
            if(CurrentWindow.WindowState == WindowState.Minimized) {
                // un minimise it (does not focus the window)
                CurrentWindow.WindowState = WindowState.Normal;
            }
            // focus the window
            CurrentWindow.Activate();

        }

        public void DataContext(object Context) {
            this.Context = Context;
            if(CurrentWindow != null) {
                CurrentWindow.DataContext = Context;
            }
        }

        // remove log from cache as the window is closed
        public void Close() { Close(null, null); }
        public void Close(object sender, EventArgs e) {
            CurrentWindow.Close();
            CurrentWindow = null;
        }
    }
}
