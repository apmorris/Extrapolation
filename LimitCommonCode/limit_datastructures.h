// Some dirt simple data structures that need to be shared, and so the defins are pulled out.
#ifndef __limit_datastructures__
#define __limit_datastructures__

#include <vector>
#include <ostream>
#include <map>
#include <string>

// Number of events, signal, etc., in the ABCD regions.
struct ABCD {
	double A, B, C, D;
};

// Allow use to refer to regions directly instead of by numbers.
enum Region {
	A = 0, B = 1, C = 2, D = 3
};

// For signal, the number of events is also associated with a lifetime.
struct signal_lifetime {
	ABCD signalEvents;
	double lifetime;
	std::vector<double> efficiency; // A, B, C, D efficiencies for the sample
};

// Control how the limit is actually run
struct abcd_limit_config {
	bool useToys; // True if we should run toys, otherwise run asym fit.
	bool scaleLimitByEfficiency; // True if we should run limit once, and rescale result. False we re-run limit at each lifetime point.
	std::string fileName; // Output filename for this
	double rescaleSignalTo; // How to rescale region A during the limit setting
	int nToys; // How many toys to run if using the toy method
        int randomSeed; // Seed for random number generator if using toys
	std::map<std::string, double> systematic_errors;
	double luminosity; // Lumi in fb that we are looking at
};

// Result (and input parameters) from a limit.
struct limit_result {
	ABCD observed_data;
	signal_lifetime signal;
	// Expected limit numbers.
	// TODO: Rename to something more clear.
	double cl_95;
	double cl_p1sigma;
	double cl_p2sigma;
	double cl_n1sigma;
	double cl_n2sigma;

	// Actual limit.
	double cl_limit;
};

// Some output operators to make debugging/dumping a little easier.

inline std::ostream &operator<< (std::ostream &s, const ABCD &v)
{
	s << "(" << v.A << ", " << v.B << ", " << v.C << ", " << v.D << ")";
	return s;
}

inline std::ostream &operator<< (std::ostream &s, const signal_lifetime &v)
{
	s << "(ctau=" << v.lifetime << ", events=" << v.signalEvents << ")";
	return s;
}

inline std::ostream &operator<< (std::ostream &s, const limit_result &r)
{
	s << "Obs: " << r.observed_data << " sig: " << r.signal
		<< " limit: " << r.cl_limit
		<< " 95%: " << r.cl_95 << " 1s+" << r.cl_p1sigma << "-" << r.cl_n1sigma
		<< " 2s+" << r.cl_p2sigma << "-" << r.cl_n2sigma;

	return s;
}

#endif
