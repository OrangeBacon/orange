using System;

public static class Program {
    [STAThread]
    public static void Main(string[] args) {
        var app = new Interface.App();
        app.InitializeComponent();
        app.Run();
    }
}

