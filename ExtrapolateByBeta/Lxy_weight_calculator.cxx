#include "Lxy_weight_calculator.h"

using namespace std;

// Defien the constants for the Lxy1 and Lxy2 binning
const int n_bins = 30; // 100 bins
const double lxy_min = 0.0; // lxy minimum in meters
const double lxy_max = 5.0; // lxy maximum in meters


//
// Calcualte the weight histogram
// Note: In Run1 this was done during the actual running on MC that generates this ntuple. But by doing it here
//       we better keep cuts consistent by keeping them close to each other.
// Note: this is not weighted, but the beta shape is (so we don't do weights twice).
Lxy_weight_calculator1D::Lxy_weight_calculator1D(const muon_tree_processor &reader)
{
	// Events where they were generated and where they passed the analysis selection
	unique_ptr<TH1D> generated(new TH1D("generated", "generated", n_bins, lxy_min, lxy_max));
	unique_ptr<TH1D> passedA(new TH1D("passedA", "passedA", n_bins, lxy_min, lxy_max));
	unique_ptr<TH1D> passedB(new TH1D("passedB", "passedB", n_bins, lxy_min, lxy_max));
	unique_ptr<TH1D> passedC(new TH1D("passedC", "passedC", n_bins, lxy_min, lxy_max));
	unique_ptr<TH1D> passedD(new TH1D("passedD", "passedD", n_bins, lxy_min, lxy_max));
	generated->Sumw2();
	passedA->Sumw2();
	passedB->Sumw2();
	passedC->Sumw2();
	passedD->Sumw2();

	reader.process_all_entries([&generated, &passedA, &passedB, &passedC, &passedD](const muon_tree_processor::eventInfo &entry) {
		generated->Fill(entry.vpi1_Lxy / 1000.0, entry.weight);
		generated->Fill(entry.vpi2_Lxy / 1000.0, entry.weight);
		if (entry.RegionA) {
			passedA->Fill(entry.vpi1_Lxy / 1000.0, entry.weight);
			passedA->Fill(entry.vpi2_Lxy / 1000.0, entry.weight);
		}
		if (entry.RegionB) {
			passedB->Fill(entry.vpi1_Lxy / 1000.0, entry.weight);
			passedB->Fill(entry.vpi2_Lxy / 1000.0, entry.weight);
		}
		if (entry.RegionC) {
			passedC->Fill(entry.vpi1_Lxy / 1000.0, entry.weight);
			passedC->Fill(entry.vpi2_Lxy / 1000.0, entry.weight);
		}
		if (entry.RegionD) {
			passedD->Fill(entry.vpi1_Lxy / 1000.0, entry.weight);
			passedD->Fill(entry.vpi2_Lxy / 1000.0, entry.weight);
		}
	});

	// The key is the ratio.
	// We can't use ROOT sumw2 errors because they assume independent histograms. So we use
	// binomial errors (the "B" option).
	_pass_weight[0] = unique_ptr<TH1D>(new TH1D("_lxy_pass_weightA", "Analysis efficiency Region A; Lxy [m]; Lxy [m]", n_bins, lxy_min, lxy_max));
	_pass_weight[0]->Divide(passedA.get(), generated.get(), 1.0, 1.0, "B");
	_pass_weight[1] = unique_ptr<TH1D>(new TH1D("_lxy_pass_weightB", "Analysis efficiency Region B; Lxy [m]; Lxy [m]", n_bins, lxy_min, lxy_max));
	_pass_weight[1]->Divide(passedB.get(), generated.get(), 1.0, 1.0, "B");
	_pass_weight[2] = unique_ptr<TH1D>(new TH1D("_lxy_pass_weightC", "Analysis efficiency Region C; Lxy [m]; Lxy [m]", n_bins, lxy_min, lxy_max));
	_pass_weight[2]->Divide(passedC.get(), generated.get(), 1.0, 1.0, "B");
	_pass_weight[3] = unique_ptr<TH1D>(new TH1D("_lxy_pass_weightD", "Analysis efficiency Region D; Lxy [m]; Lxy [m]", n_bins, lxy_min, lxy_max));
	_pass_weight[3]->Divide(passedD.get(), generated.get(), 1.0, 1.0, "B");
}
Lxy_weight_calculator2D::Lxy_weight_calculator2D(const muon_tree_processor &reader)
{
	// Events where they were generated and where they passed the analysis selection
	unique_ptr<TH2D> generated(new TH2D("generated", "generated", n_bins, lxy_min, lxy_max, n_bins, lxy_min, lxy_max));
	unique_ptr<TH2D> passedA(new TH2D("passedA", "passedA", n_bins, lxy_min, lxy_max, n_bins, lxy_min, lxy_max));
	unique_ptr<TH2D> passedB(new TH2D("passedB", "passedB", n_bins, lxy_min, lxy_max, n_bins, lxy_min, lxy_max));
	unique_ptr<TH2D> passedC(new TH2D("passedC", "passedC", n_bins, lxy_min, lxy_max, n_bins, lxy_min, lxy_max));
	unique_ptr<TH2D> passedD(new TH2D("passedD", "passedD", n_bins, lxy_min, lxy_max, n_bins, lxy_min, lxy_max));
	generated->Sumw2();
	passedA->Sumw2();
	passedB->Sumw2();
	passedC->Sumw2();
	passedD->Sumw2();

	reader.process_all_entries([&generated, &passedA, &passedB, &passedC, &passedD](const muon_tree_processor::eventInfo &entry) {
		generated->Fill(entry.vpi1_Lxy / 1000.0, entry.vpi2_Lxy / 1000.0, entry.weight);
		if (entry.RegionA) {
			passedA->Fill(entry.vpi1_Lxy / 1000.0, entry.vpi2_Lxy / 1000.0, entry.weight);
		}
		if (entry.RegionB) {
			passedB->Fill(entry.vpi1_Lxy / 1000.0, entry.vpi2_Lxy / 1000.0, entry.weight);
		}
		if (entry.RegionC) {
			passedC->Fill(entry.vpi1_Lxy / 1000.0, entry.vpi2_Lxy / 1000.0, entry.weight);
		}
		if (entry.RegionD) {
			passedD->Fill(entry.vpi1_Lxy / 1000.0, entry.vpi2_Lxy / 1000.0, entry.weight);
		}
	});

	// Smooth everything
	passedA->Smooth();
	passedB->Smooth();
	passedC->Smooth();
	passedD->Smooth();

	// The key is the ratio.
	// We can't use ROOT sumw2 errors because they assume independent histograms. So we use
	// binomial errors (the "B" option).
	_pass_weight[0] = unique_ptr<TH2D>(new TH2D("_lxy_pass_weightA", "Analysis efficiency Region A; Lxy [m]; Lxy [m]", n_bins, lxy_min, lxy_max, n_bins, lxy_min, lxy_max));
	_pass_weight[0]->Divide(passedA.get(), generated.get(), 1.0, 1.0, "B");
	_pass_weight[1] = unique_ptr<TH2D>(new TH2D("_lxy_pass_weightB", "Analysis efficiency Region B; Lxy [m]; Lxy [m]", n_bins, lxy_min, lxy_max, n_bins, lxy_min, lxy_max));
	_pass_weight[1]->Divide(passedB.get(), generated.get(), 1.0, 1.0, "B");
	_pass_weight[2] = unique_ptr<TH2D>(new TH2D("_lxy_pass_weightC", "Analysis efficiency Region C; Lxy [m]; Lxy [m]", n_bins, lxy_min, lxy_max, n_bins, lxy_min, lxy_max));
	_pass_weight[2]->Divide(passedC.get(), generated.get(), 1.0, 1.0, "B");
	_pass_weight[3] = unique_ptr<TH2D>(new TH2D("_lxy_pass_weightD", "Analysis efficiency Region D; Lxy [m]; Lxy [m]", n_bins, lxy_min, lxy_max, n_bins, lxy_min, lxy_max));
	_pass_weight[3]->Divide(passedD.get(), generated.get(), 1.0, 1.0, "B");
}

Lxy_weight_calculator1D::~Lxy_weight_calculator1D()
{
}
Lxy_weight_calculator2D::~Lxy_weight_calculator2D()
{
}

// Do the lookup to make the calculation
double Lxy_weight_calculator1D::operator()(int region, double lxy1, double lxy2) const
{
	if (region < 0 || region > 3) {
		throw runtime_error("Illegal region in operator()");
	}
	int xbin1 = _pass_weight[region]->FindBin(lxy1);
	int xbin2 = _pass_weight[region]->FindBin(lxy2);
	return sqrt(_pass_weight[region]->GetBinContent(xbin1) * _pass_weight[region]->GetBinContent(xbin2));
}

double Lxy_weight_calculator2D::operator()(int region, double lxy1, double lxy2) const
{
	if (region < 0 || region > 3) {
		throw runtime_error("Illegal region in operator()");
	}
	int xbin1 = _pass_weight[region]->FindBin(lxy1, lxy2);
	return _pass_weight[region]->GetBinContent(xbin1);
}
