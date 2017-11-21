//
// Given an extrapolation file as input, derive the limits as a function of
// lifetime for output.
//

#include "extrap_file_wrapper.h"
#include "limitSetting.h"

#include "Wild/CommandLine.h"
#include "TApplication.h"
#include "TROOT.h"

#include <iostream>
#include <memory>
#include <string>

using namespace std;
using namespace Wild::CommandLine;

// Helper functions, etc.
struct config {
	string extrapolate_filename; // Input ROOT file to guide this whole thing

	ABCD observed_data;
	abcd_limit_config limit_settings;
};
config parse_command_line(int argc, char **argv);

// Main entry point
int main(int argc, char **argv)
{
	// Get ROOT setup so we don't have funny problems with it later.
	int dummy_argc = 0;
	auto a = unique_ptr<TApplication>(new TApplication("extrapolate_betaw", &dummy_argc, argv));
	//gROOT->SetBatch(kTRUE);

	// Protect against anything weird going wrong so we get a sensible error message.
	try {
		auto c = parse_command_line(argc, argv);
		extrap_file_wrapper input_file(c.extrapolate_filename);

		// If we are going to do it by setting the limit once and calculating the efficiency.
		if (c.limit_settings.scaleLimitByEfficiency) {
			extrapolate_limit_to_lifetime_by_efficency (input_file, c.observed_data, c.limit_settings);
		}
		else {
			extrapolate_limit_to_lifetime (input_file, c.observed_data, c.limit_settings);
		}
	}
	catch (exception &e) {
		cout << "Total failure - exception thrown: " << e.what() << endl;
		return 1;
	}
	return 0;
}

// Fill in sys errors for a particular scalar mass. Bomb if we don't have them.
void fill_sys_errors(int scalarMass, map<string, double> &errors, bool unofficial = false)
{
	double jes, jesemf, jer, trigger, pileup;
	switch (scalarMass) {
	case 125:
	  jes = 0.037;
	  jesemf = 0.173;
	  jer = 0.155;
	  trigger = 0.045;
	  pileup = 0.047;
	  break;

	case 200:
	  jes = 0.051;
	  jesemf = 0.096;
	  jer = 0.015;
	  trigger = 0.058;
	  pileup = 0.005;
	  break; 

	case 400:
	  jes = 0.01;
	  jesemf = 0.033;
	  jer = 0.016;
	  trigger = 0.017;
	  pileup = 0.047;
	  break;

	case 600:
	  jes = 0.006;
	  jesemf = 0.01;
	  jer = 0.011;
	  trigger = 0.011;
	  pileup = 0.019;
	  break;

	case 1000:
	  jes = 0.009;
	  jesemf = 0.007;
	  jer = 0.01;
	  trigger = 0.007;
	  pileup = 0.027;
	  break;

	default:
		if (unofficial) {
			// Use 400 for now
			cout << "*** -> Unknown systematic errors for scalar mass " << scalarMass << " - using 400 GeV" << endl;
			jes = 0.01;
			jesemf = 0.033;
			jer = 0.0016;
			trigger = 0.017;
			pileup = 0.047;
		break;
		}
		else {
			throw runtime_error("Do not know systematic errors for the scalar mass!");
		}
		break;
	}

	// Store them.
	errors["mc_jes"] = jes;
	errors["mc_jesemf"] = jesemf;
	errors["mc_jer"] = jer;
	errors["mc_trigger"] = trigger;
	errors["mc_pileup"] = pileup;
}

// Find all errors that start with some prefix, and sum them.
double budle_errors(const string &prefix, const map<string, double> &errors)
{
	double err2 = 0.0;
	for (auto const &err : errors) {
		if (err.first.find(prefix) == 0) {
			err2 += err.second*err.second;
		}
	}
	return sqrt(err2);
}

void dump_errors(const map<string, double> &errors) {
	cout << "**********" << endl;
	cout << "* Systematic Errors: " << endl;
	cout << "* " <<endl;

	for (auto &err : errors) {
		cout << "* " << err.first << " -> " << err.second << endl;
	}

	cout << "* " << endl;
	cout << "**********" << endl;
}


// Parse command line arguments
config parse_command_line(int argc, char **argv)
{
	// Setup the command line arguments
	Args args({
		// The input MC file which contains just about all the info we could need from MC.
                Arg("extrapFile", "e", "The extrapolation root file", Is::Required),

		// We need to know how many data were in each of the A, B, C, and D regions.
                Arg("nA", "A", "How many events observed in data in region A", Is::Required),
		Arg("nB", "B", "How many events observed in data in region B", Is::Required),
		Arg("nC", "C", "How many events observed in data in region C", Is::Required),
		Arg("nD", "D", "How many events observed in data in region D", Is::Required),

		Arg("Luminosity", "L", "Lumi, in fb, for this dataset", Is::Optional),
		Arg("ABCDError", "E", "Error on the ABCD component", Is::Optional),
		Arg("MCErrorScale", "M", "Factor to scale the MC error by", Is::Optional),

		// Output options
		Arg("OutputFile", "f", "Name of output root file for limit results. By default based on input filename", Is::Optional),

		// How is the limit set?
		Flag("UseAsym", "a", "Do asymtotic fit rather than using toys (toys are slow!)"),
		Flag("ExtrapAtEachLifetime", "l", "Refit limit at each lifetime point to take into account differing efficiencies at A, B, C and D"),
		Arg("RescaleSignal", "r", "Rescale the expected signal in region A to this number during limit setting", Is::Optional),
		Arg("NToys", "n", "Number of toys to use when using toy method. Defaults to 5000.", Is::Optional),

		// General
		Flag("Unofficial", "u", "Turn off some protection checks so it can run even thought input isn't 'just right'"),
	});

	// Make sure we got all the command line arguments we need
	if (argc == 1 || !args.Parse(argc, argv)) {
		cout << args.Usage("ExtrapLimitFinder") << endl;
		throw runtime_error("Bad command line arguments - exiting");
	}

	// Fill up the config guy and return it.
	config result;
	result.extrapolate_filename = args.Get("extrapFile");

	result.observed_data.A = args.GetAsFloat("nA");
	result.observed_data.B = args.GetAsFloat("nB");
	result.observed_data.C = args.GetAsFloat("nC");
	result.observed_data.D = args.GetAsFloat("nD");

	result.limit_settings.useToys = !args.IsSet("UseAsym");
	result.limit_settings.scaleLimitByEfficiency = !args.IsSet("ExtrapAtEachLifetime");


	auto baseFilename = result.extrapolate_filename.rfind("\\") == string::npos
		? result.extrapolate_filename
		: result.extrapolate_filename.substr(result.extrapolate_filename.rfind("\\") + 1);
	result.limit_settings.fileName = args.IsSet("OutputFile")
		? args.Get("OutputFile")
		: string("limit_result_") + baseFilename;
	result.limit_settings.rescaleSignalTo = args.IsSet("RescaleSignal")
		? args.GetAsFloat("RescaleSignal")
		: 0.0;
	result.limit_settings.nToys = args.IsSet("NToys")
		? args.GetAsInt("NToys")
		: 5000;
	result.limit_settings.luminosity = args.IsSet("Luminosity")
		? args.GetAsFloat("Luminosity")
		: 33.0;

	// Systematic Errors
	result.limit_settings.systematic_errors["lumi"] = 0.022; // Final lumi is 2.1%

	// Constant MC errors.
	//result.limit_settings.systematic_errors["mc_ISRFSR"] = 0.02;

	// Constant ABCD errors
	result.limit_settings.systematic_errors["abcd"] = args.IsSet("ABCDError")
		? args.GetAsFloat("ABCDError")
		: 0.36;

	// Next we have to determine what mass point we are looking at. This is required as the
	// systematic errors do vary with mass.
	int scalarMass = result.extrapolate_filename.find("mH100_") != string::npos ? 100
		: result.extrapolate_filename.find("mH125_") != string::npos ? 125
		: result.extrapolate_filename.find("mH200_") != string::npos ? 200
		: result.extrapolate_filename.find("mH400_") != string::npos ? 400
		: result.extrapolate_filename.find("mH600_") != string::npos ? 600
		: result.extrapolate_filename.find("mH1000_") != string::npos ? 1000
		: -1;

	if (scalarMass <= 0.0) {
		cout << "*******" << endl;
		cout << "*" << endl;
		cout << "* -> Unable to determine a scalar mass from the input file name: " << endl;
		cout << "*     -> " << result.extrapolate_filename << endl;
		cout << "*" << endl;
		cout << "* Going to fail unless this is an unoffical run!" << endl;
		cout << "*" << endl;
		cout << "*******" << endl;
		if (!args.IsSet("Unofficial")) {
			throw runtime_error("Unable to determine the scalar mass from the input file " + result.extrapolate_filename);
		}
		cout << "**** -> Setting scalar mass to 400.0" << endl;
		scalarMass = 400;
	}

	auto mcScale = args.IsSet("MCErrorScale")
	  ? args.GetAsFloat("MCErrorScale")
	  : 1.0;

	fill_sys_errors(scalarMass, result.limit_settings.systematic_errors);
	result.limit_settings.systematic_errors["mc_eff"] = budle_errors("mc_", result.limit_settings.systematic_errors)*mcScale;

	// Finally, dump them.
	dump_errors(result.limit_settings.systematic_errors);

	return result;
}

