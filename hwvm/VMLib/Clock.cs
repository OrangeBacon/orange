using System.Collections.Generic;
using System.Linq;

namespace VMLib {
    // system clock, controls execution of all logic in the system
    public class Clock {

        // function to be run
        public delegate void ClockEventFunc();

        // handlers to run at each clock phase
        private readonly List<ClockEvent> handlers = new List<ClockEvent>();

        // cleanup events
        private readonly ClockEvent end = new ClockEvent();

        // get the clockevent at a specific time so a handler can be added
        public ClockEvent At(int time) {
            if(handlers.ElementAtOrDefault(time) != null) {
                // return existing clockevent
                return handlers.ElementAt(time);
            }
            
            // make handlers long enough so element at does not fail
            // (Insert tried, however that fails if list not long enough)
            while(handlers.Count <= time) {
                handlers.Add(new ClockEvent());
            }
            return handlers.ElementAt(time);
        }

        public ClockEvent AtEnd() {
            return end;
        }

        // container for all handlers that run at the same time
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

        // Run one complete system cycle,
        // equivalent to what happens on rising edge of electrical
        // clock signal
        public void RunCycle() {
            foreach(var handler in handlers) {
                handler.Run();
            }
            end.Run();
        }
    }
}