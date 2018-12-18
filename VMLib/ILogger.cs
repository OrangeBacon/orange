using System;
using System.Collections.Generic;
using System.Text;

namespace VMLib {
    public interface ILogger {
        void Info(String message);
        void Warn(String message);
        void Error(String message);
    }
}
