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
        public MainWindow() {
            InitializeComponent();
            var logger = new Logger(LogList);
            var vm = new StarfishVM(logger).VM;
            logger.Info(vm.Components[0].ComponentID.ToString());
            ComponentsList.ItemsSource = new List<Component>(vm.Components);
            logger.Info(ComponentsList.ItemsSource.ToString());
            CommandList.ItemsSource = new List<Component>(vm.Components);
        }
    }
}
