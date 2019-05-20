using System.Windows.Controls;
using System.Windows;

namespace Interface.Views {

    public partial class MemoryViewer : Window {
        public MemoryViewer() {
            InitializeComponent();
            Closed += (sender, e) => {
                if(DataContext.GetType() == typeof(ViewModels.MemoryViewer)) {
                    ((ViewModels.MemoryViewer)DataContext).Scroll = null;
                }
            };
        }

        private void Scroll_Loaded(object sender, RoutedEventArgs e) {
            if(DataContext.GetType() == typeof(ViewModels.MemoryViewer)) {
                ((ViewModels.MemoryViewer)DataContext).Scroll = (ScrollViewer)sender;
            }
        }
    }
}
