#ifndef __SimulABCD__
#define __SimulABCD__

#include "limit_datastructures.h"
#include "HypoTestInvTool.h"

#include "TROOT.h"

#include <iostream>
#include <string>
#include <map>

/*
Two variables x,y create 4 regions: A,B,C,D with A signal dominated region and
B,C,D control regions (sidebands).

^ y (sumpt)
|
|--------------------------+
|    C         |     B     |
|              |           |
|              |           |
|              |           |
|              |           |
|--------------+-----------|
|    D         |     A     |
|              |           |
|              |  Signal   |
|              |           |
+-----------------------------> x (deltaPhi)
^

ABCD Ansatz A:B = D:C -->  A = B * D/C

*/

HypoTestInvTool::LimitResults simultaneousABCD(const Double_t n[4], const Double_t s[4], const Double_t b[4], const Double_t c[4],
	TString out_filename = "ABCD_ws.root",
	Bool_t useB = kFALSE, // Use background as estimated in MC
	Bool_t useC = kFALSE, // Use other background events (do subtraction of c above
	Bool_t blindA = kTRUE, // Assume no signal, so we get expected limits
	Int_t calcType = 0, // 0 for toys, 2 for asym fit
	Int_t par_ntoys = 5000, // Number of toys in dataset.
        Int_t randomSeed = 4378, // Random seed
	std::map<std::string,double> systematic_errors = std::map<std::string, double>() // Errors that are needed for the limit
);

inline HypoTestInvTool::LimitResults simultaneousABCD(const std::vector<double> &n,
	const std::vector<double> &s,
	const std::vector<double> &b,
	const std::vector<double> &c,
	TString out_filename = "ABCD_ws.root",
	Bool_t useB = kFALSE, // Use background as estimated in MC
	Bool_t useC = kFALSE, // Use other background events (do subtraction of c above
	Bool_t blindA = kTRUE, // Assume no signal, so we get expected limits
	Int_t calcType = 0, // 0 for toys, 2 for asym fit
	Int_t par_ntoys = 5000,// Number of toys to throw
        Int_t randomSeed = 4378, // Random seed
	const std::map<std::string, double> &systematic_errors = std::map<std::string, double>()
)
{
	if (n.size() != 4
		|| s.size() != 4
		|| b.size() != 4
		|| c.size() != 4) {
		throw std::runtime_error("n, s, b, and c must have 4 elements exactly");
	}

	return simultaneousABCD(&(n[0]), &(s[0]), &(b[0]), &(c[0]),
		out_filename, useB, useC, blindA, calcType, par_ntoys, randomSeed,
		systematic_errors);
}

// Convert to a vector that we can pass to the simultanious fitter.
// TODO: Move this and do_abcd_limit out of this file to prevent confusion.
inline std::vector<double> ABCD_as_vector_CalRToLJ(const ABCD &events)
{
	std::vector<double> n;
	n.push_back(events.A);
	n.push_back(events.B);
	n.push_back(events.D);
	n.push_back(events.C);
	return n;
}

// Rescale the number of events so that regionA is the rescaleTo value. If the rescale number
// is 0.0, then just leave them.
inline ABCD rescale_events_in_regionA(const ABCD &events, double rescaleTo) {
	if (rescaleTo <= 0.0)
		return events;

	auto result = events;
	auto factor = rescaleTo / events.A;

	result.A *= factor;
	result.B *= factor;
	result.C *= factor;
	result.D *= factor;

	return result;
}

// Calculate the limit, and fill in all the results, and return it.
// Note that the conversion between LJ and CalR world is done here!
inline limit_result do_abcd_limit(const ABCD &data, const signal_lifetime &expected_signal, const abcd_limit_config &config)
{
	std::vector<double> dummy(4);
	fill(dummy.begin(), dummy.end(), 0.0);

	auto rescaled_expected_signal = expected_signal;
	rescaled_expected_signal.signalEvents = rescale_events_in_regionA(expected_signal.signalEvents, config.rescaleSignalTo);

	auto calc_filename = std::string("limit_calc_") + config.fileName;

	auto limit = simultaneousABCD(ABCD_as_vector_CalRToLJ(data), ABCD_as_vector_CalRToLJ(rescaled_expected_signal.signalEvents),
		dummy, dummy,
		calc_filename,
		false, false,
		data.A == 0,
		config.useToys ? 0 : 2,
                config.nToys, config.randomSeed,
		config.systematic_errors);

	std::cout << "Limit. data: " << data << "  expected signal: " << rescaled_expected_signal << std::endl;
	std::cout << "  -> " << limit << std::endl;

	auto mu_scale = expected_signal.signalEvents.A / rescaled_expected_signal.signalEvents.A;

	limit_result r;
	r.cl_p1sigma = limit.sigma_plus_1 * mu_scale;
	r.cl_p2sigma = limit.sigma_plus_2 * mu_scale;
	r.cl_n1sigma = limit.sigma_minus_1 * mu_scale;
	r.cl_n2sigma = limit.sigma_minus_2 * mu_scale;
	r.cl_95 = limit.median * mu_scale;
	r.cl_limit = limit.upper_limit * mu_scale;
	r.signal = expected_signal;
	r.observed_data = data;

	std::cout << "Limit rescaled: " << r << std::endl;

	return r;
}
#endif

