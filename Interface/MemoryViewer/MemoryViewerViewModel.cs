using System;
using System.Collections;
using System.Collections.ObjectModel;
using System.Linq;
using VMLib;

namespace Interface.ViewModels {
    internal class MemoryViewer {
        public ushort[] IndexList { get; } = new ushort[ushort.MaxValue];
        public ushort[] Memory { get; private set; }

        public Pair<ushort, MemoryViewModel>[] IndexMemoryPair { get; private set; } = new Pair<ushort, MemoryViewModel>[ushort.MaxValue];

        public MemoryViewer(VMCore core) {
            Memory = core.Memory.Memory;
            for(ushort i = 0; i < IndexList.Length; i++) {
                IndexList[i] = i;
                IndexMemoryPair[i] = new Pair<ushort, MemoryViewModel>(i, new MemoryViewModel(Memory, i));
            }
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
        public ushort[] Memory { get; set; }

        public BitModel(int index, int bitIndex, bool isChecked, ushort[] Memory) {
            Index = index;
            BitIndex = bitIndex;
            this.isChecked = isChecked;
            this.Memory = Memory;
        }

        private bool isChecked;
        public bool IsChecked {
            get => isChecked;
            set {
                if(value) {
                    Memory[Index] |= (ushort)(1 << BitIndex);
                } else {
                    Memory[Index] &= (ushort)~(1 << BitIndex);
                }
                isChecked = value;
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

        public MemoryViewModel(ushort[] Memory, ushort index) {
            Items = new ObservableCollection<BitModel>();
            for(int i = 0; i < 16; i++) {
                Items.Add(new BitModel(index, i, ((Memory[i] >> i)&1) != 0, Memory));
            }
        }
    }
}
