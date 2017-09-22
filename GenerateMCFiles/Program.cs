using CommandLine;
using libDataAccess.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using static System.Console;
using static GenerateMCFiles.GeneratorManager;

namespace GenerateMCFiles
{
    /// <summary>
    /// For a given set of MC files, generate output files for everything:
    ///  - The beta info for all MC files
    ///  - The efficiency plot used to calculate the propagation
    /// </summary>
    class Program
    {
        class Options : CommandLineUtils.CommonOptions
        {
            [Value(1, MetaName = "Datasets", Required = true, HelpText = "List of dataset names that we should process")]
            public IEnumerable<string> Datasets { get; set; }

            [Option("JobName", Default = "DiVertAnalysis", Required = false, HelpText = "Job name we should fetch for the datasets")]
            public string JobName { get; set; }

            [Option("JobVersion", Default = 8, Required =false, HelpText = "Job version number.")]
            public int JobVersion { get; set; }
        }

        static void Main(string[] args)
        {
            // Parse the command parameters
            var opt = CommandLineUtils.ParseOptions<Options>(args);

            libDataAccess.Files.JobVersionNumber = opt.JobVersion;
            libDataAccess.Files.JobName = opt.JobName;

            // Next, for each dataset, write out the files.
            foreach (var ds in opt.Datasets)
            {
                WriteLine($"Looking at {ds}.");
                GenerateExtrapolationMCFiles(ds); 
            }

            FutureConsole.DumpToCout();
        }

    }
}
