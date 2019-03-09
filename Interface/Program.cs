using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public static class Program {
    [STAThread]
    public static void Main(string[] args) {
        var app = new Interface.App();
        app.InitializeComponent();
        app.Run();
    }
}

