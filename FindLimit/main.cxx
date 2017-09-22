///
/// Command line run of the limit setting. This is a tool mostly used for testing.
///

// There is potential confusion on how A, B, C, and D are defined. This is because the code that drives
// this comes from the Lepton-Jets analysis which uses a different setup. We use the CalRatio definition.
//
/*
^ y(sumBDT)
|
|--------------------------+
|    B         |     A     |
|              |           |
|              |           |
|              |           |
|              |           |
|--------------+-----------|
|    D         |     C     |
|              |           |
|              |  Signal   |
|              |           |
+----------------------------->x(sumDRMax2GeV)

ABCD Ansatz A:C = B:D -->  A = C * B/D
^
*/

#include "SimulABCD.h"
#include "CalRLJConverter.h"

#include "TApplication.h"

#include <memory>
#include <iostream>

#include "Wild/CommandLine.h"

using namespace std;
using namespace Wild::CommandLine;

// Helper
struct limit_config {
	// Observed in data.
	// A <= 0 for expected limit (e.g. blind).
	double A, B, C, D;

	// Expected in signal
	double sA, sB, sC, sD;

	bool useToys;
};

limit_config parse_command_line(int argc, char **argv);

// Main entry point
int main(int argc, char **argv)
{
	int dummy_argc = 0;
	auto a = make_unique<TApplication>("extrapolate_betaw", &dummy_argc, argv);
	//gROOT->SetBatch(true);
	try {
		// Pull out the various command line arguments
		auto config = parse_command_line(argc, argv);

		// Get this into a form the limit setter likes.
		double data[] = { config.A, config.B, config.C, config.D };
		double signal[] = { config.sA, config.sB, config.sC, config.sD };
		
		auto blinded = config.A <= 0;

		// And call it.
		double dummy[4];
		simultaneousABCD(ConvertFromCalRToLJ(data),
			ConvertFromCalRToLJ(signal),
			ConvertFromCalRToLJ(dummy),
			ConvertFromCalRToLJ(dummy),
			"limit_result.root", false, false,
			blinded,
			config.useToys ? 0 : 2);
		
		return 0;
	}
	catch (exception &e) {
		cout << "Error during running: " << e.what() << endl;
	}
}

// Parse the command line options and create the config stuff.
limit_config parse_command_line(int argc, char **argv)
{
	// Setup the command line arguments
	Args args({
		// Arguments that must be present on the command line, one after the other
		Arg("nA", "A", "Number of events observed in A.", Ordinality::Required),
		Arg("nB", "B", "Number of events observed in B.", Ordinality::Required),
		Arg("nC", "C", "Number of events observed in C.", Ordinality::Required),
		Arg("nD", "D", "Number of events observed in D.", Ordinality::Required),

		Arg("sA", "w", "Number of signal events expected in A.", Ordinality::Required),
		Arg("sB", "x", "Number of signal events expected in B.", Ordinality::Required),
		Arg("sC", "y", "Number of signal events expected in C.", Ordinality::Required),
		Arg("sD", "z", "Number of signal events expected in D.", Ordinality::Required),
		Flag("UseAsym", "a", "Do asymtotic fit rather than using toys")
	});

	// Make sure we got all the command line arguments we need
	if (argc == 1 || !args.Parse(argc, argv)) {
		cout << args.Usage("PlotSingleLimit") << endl;
		cout << "  -> A, B, C, and D are defined as they are for the CalRatio analysis!" << endl;
		throw runtime_error("Bad command line arguments - exiting");
	}

	limit_config result;
	// Get the A, B, C, and D fellows.
	result.A = args.GetAsFloat("nA");
	result.B = args.GetAsFloat("nB");
	result.C = args.GetAsFloat("nC");
	result.D = args.GetAsFloat("nD");

	result.sA = args.GetAsFloat("sA");
	result.sB = args.GetAsFloat("sB");
	result.sC = args.GetAsFloat("sC");
	result.sD = args.GetAsFloat("sD");

	result.useToys = !args.IsSet("UseAsym");

	// Done!
	return result;
}
