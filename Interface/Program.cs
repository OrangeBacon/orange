using Squirrel;
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

        using(var mgr = UpdateManager.GitHubUpdateManager(@"https://github.com/ScratchOs/starfish", prerelease: true)) {
            Interface.Framework.Globals.Log.Info("Starting Update Check");
            mgr.Result.UpdateApp();
        }
    }
}

