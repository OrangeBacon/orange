using VMLib;

namespace Interface.ViewModels {
    internal class MemoryViewer {
        public ushort[] IndexList { get; } = new ushort[ushort.MaxValue];
        public ushort[] Memory { get; private set; }

        public Pair<ushort, ushort>[] IndexMemoryPair { get; private set; } = new Pair<ushort, ushort>[ushort.MaxValue];

        public MemoryViewer(VMCore core) {
            Memory = core.Memory.Memory;
            for(ushort i = 0; i < IndexList.Length; i++) {
                IndexList[i] = i;
                IndexMemoryPair[i] = new Pair<ushort, ushort>(i, Memory[i]);
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
}
