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
using Interface.Framework;

namespace Interface.Views {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MicrocodeProcess : Window {
        VMCore vm;
        private ILogger logger;

        public MicrocodeProcess() {
            InitializeComponent();

            logger = (ViewModels.App)Application.Current.Properties[ViewModels.App.TypeName];

            vm = new StarfishVM(logger).VM;
            ComponentsList.ItemsSource = vm.Components;

            CommandList.ItemsSource = vm.Components;

            ((ViewModels.MicrocodeProcess)DataContext).ShowLogEvent += ShowLog;
        }

        private void ShowLog(object sender, EventArgs e) {
            var win = new Log {
                Owner = this
            };
            win.Show();
        }

        private void SingleStep(object sender, RoutedEventArgs e) {
            logger.Info("STEP");
            foreach (Component component in vm.Components) {
                foreach (MicrocodeCommand command in component.Commands) {
                    if (command.Active) {
                        logger.Info(command.Name);
                    }
                }
            }
            e.Handled = true;
            logger.Info(e.Source.ToString());
        }
    }
}
