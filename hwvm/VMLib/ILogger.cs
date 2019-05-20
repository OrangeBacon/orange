using System;

namespace VMLib {

    // generic log interface
    public interface ILogger {
        void Info(string message);
        void Warn(string message);
        void Error(string message);
    }
}
