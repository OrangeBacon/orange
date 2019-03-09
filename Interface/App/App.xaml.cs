using System.Windows;

namespace Interface {

    public partial class App : Application {
        private void Application_Startup(object sender, StartupEventArgs e) {
            // when the application starts, the main view should be opened
            var win = new Views.MicrocodeProcess {
                Title = "Starfish Virtual Machine"
            };
            win.Show();
        }
    }
}
