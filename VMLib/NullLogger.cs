using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    // Logger class that does nothing
    class NullLogger : ILogger {
        public void Error(string message) {
        }

        public void Info(string message) {
        }

        public void Warn(string message) {
        }
    }
}
