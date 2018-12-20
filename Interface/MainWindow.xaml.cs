using System;
using System.Collections.Generic;
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

namespace Interface {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {
        VMCore vm;
        private Logger logger;

        public MainWindow() {
            InitializeComponent();
            logger = new Logger(LogList);
            vm = new StarfishVM(logger).VM;

            ComponentsList.ItemsSource = vm.Components;

            CommandList.ItemsSource = vm.Components;
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
