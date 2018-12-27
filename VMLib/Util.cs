using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public static class Util {
        public delegate TResult TwoArgs<T, TResult>(T arg);
        public static Func<TResult> Bind<T, TResult>(TwoArgs<T, TResult> func, T arg) {
            return () => func(arg);
        }

        public delegate void TwoArgsNoRet<T>(T arg);
        public static Action Bind<T>(TwoArgsNoRet<T> func, T arg) {
            return () => func(arg);
        }

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
        public static LogicState<T> ThreeState<T>(T val) {
            return new LogicState<T> { Value = val, State = LogicState.In | LogicState.Out };
        }
    }
}
