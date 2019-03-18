using System.Windows.Controls;
using System.Windows;

namespace Interface.Views {

    public partial class MemoryViewer : Window {
        public MemoryViewer() {
            InitializeComponent();
            DataContextChanged += (sender,e) => {
                if(DataContext.GetType() == typeof(ViewModels.MemoryViewer)){
                    ((ViewModels.MemoryViewer)DataContext).VSP = (VirtualizingStackPanel)IC.ItemsPanel.LoadContent();
                }
            };
            Closed += (sender, e) => {
                if(DataContext.GetType() == typeof(ViewModels.MemoryViewer)) {
                    ((ViewModels.MemoryViewer)DataContext).VSP = null;
                }
            };
        }
    }
}
