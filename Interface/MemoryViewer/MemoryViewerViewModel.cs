using System.Collections.ObjectModel;
using System.Windows.Controls;
using VMLib;

namespace Interface.ViewModels {
    internal class MemoryViewer : ObservableObject {
        public ushort[] IndexList { get; } = new ushort[ushort.MaxValue];
        public ObservableCollection<ushort> Memory { get; private set; }

        public Pair<ushort, MemoryViewModel>[] IndexMemoryPair { get; private set; } = new Pair<ushort, MemoryViewModel>[ushort.MaxValue];

        private readonly VMCore core;
        public ScrollViewer Scroll = null;

        public MemoryViewer(VMCore core) {
            this.core = core;
            Memory = core.Memory.Memory;
            for(ushort i = 0; i < IndexList.Length; i++) {
                IndexList[i] = i;
                IndexMemoryPair[i] = new Pair<ushort, MemoryViewModel>(i, new MemoryViewModel(Memory, i, core));
            }

            core.Memory.PropertyChanged += (sender, e) => {
                if(Scroll != null) {
                    Framework.Globals.Log.Info($"Mem Update: {Scroll.VerticalOffset}, {Scroll.ViewportHeight}");
                    for(int i = 0; i < Scroll.ViewportHeight; i++) {
                        IndexMemoryPair[(int)(Scroll.VerticalOffset + i)].Second.Update();
                    }
                }
            };
        }

        public class Pair<A, B> {
            public A First { get; private set; }
            public B Second { get; private set; }
            public Pair(A first, B second) {
                First = first;
                Second = second;
            }
        }
    }

    internal class BitModel : ObservableObject {
        // which bit in the array this controls
        public int Index { get; set; }
        public int BitIndex { get; set; }

        // the bit aray containing this bit
        public ObservableCollection<ushort> Memory { get; set; }

        public BitModel(ObservableCollection<ushort> Memory, int index, int bitIndex) {
            Index = index;
            BitIndex = bitIndex;
            this.Memory = Memory;
        }

        internal void Update() {
            OnPropertyChanged("IsChecked");
        }

        public bool IsChecked {
            get {
                return (Memory[Index] & (1 << BitIndex)) != 0;
            }
            set {
                if(value) {
                    Memory[Index] |= (ushort)(1 << BitIndex);
                } else {
                    Memory[Index] &= (ushort)~(1 << BitIndex);
                }
                OnPropertyChanged();
            }
        }
    }

    internal class MemoryViewModel : ObservableObject {

        private ObservableCollection<BitModel> items;
        public ObservableCollection<BitModel> Items {
            get => items;
            set {
                items = value;
                OnPropertyChanged(nameof(Items));
            }
        }

        public MemoryViewModel(ObservableCollection<ushort> Memory, ushort index, VMCore core) {
            Items = new ObservableCollection<BitModel>();
            for(int i = 0; i < 16; i++) {
                Items.Add(new BitModel(Memory, index, i));
            }
        }

        public void Update() {
            foreach(var item in Items) {
                item.Update();
            }
        }
    }
}
