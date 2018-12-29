using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class Clock {
        public delegate void ClockEventFunc();

        private readonly List<ClockEvent> handlers = new List<ClockEvent>();
        private readonly ClockEvent end = new ClockEvent();

        public ClockEvent At(int time) {
            if(handlers.ElementAtOrDefault(time) != null) {
                return handlers.ElementAt(time);
            }
            while(handlers.Count <= time) {
                handlers.Add(new ClockEvent());
            }
            return handlers.ElementAt(time);
        }
        public ClockEvent AtEnd() {
            return end;
        }

        public class ClockEvent {
            public List<ClockEventFunc> handlers = new List<ClockEventFunc>();
            public void Add(ClockEventFunc f) {
                handlers.Add(f);
            }
            public void Run() {
                foreach(var func in handlers) {
                    func();
                }
            }
        }

        public void RunCycle() {
            foreach(var handler in handlers) {
                handler.Run();
            }
            end.Run();
        }
    }
}