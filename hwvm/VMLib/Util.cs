using System;

namespace VMLib {
    // generic utilities that do not fit in another class
    public static class Util {

        // attatch argument to function
        public delegate void TwoArgsNoRet<T>(T arg);
        public static Action Bind<T>(TwoArgsNoRet<T> func, T arg) {
            return () => func(arg);
        }

        // input? output? both?
        [Flags]
        public enum LogicState {
            In = 1, Out = 2
        }
        public class LogicState<T> {
            public T Value;
            public LogicState State;
        }
        public static LogicState<T> InState<T>(T val) {
            return new LogicState<T> { Value = val, State = LogicState.In };
        }
        public static LogicState<T> OutState<T>(T val) {
            return new LogicState<T> { Value = val, State = LogicState.Out };
        }
        public static LogicState<T> InOutState<T>(T val) {
            return new LogicState<T> { Value = val, State = LogicState.In | LogicState.Out };
        }
    }
}
