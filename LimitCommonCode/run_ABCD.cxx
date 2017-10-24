// COntinuous Likelihood-ABCD
// author: Stefano Giagu <stefano.giagu@cern.ch>
// Version: 4.0 (cleaned up as demo) June 27th, 2016
// Modified by G. Watts for the CalRatio analysis

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

#include "HypoTestInvTool.h"
#include "SimulABCD.h"

#include <TFile.h>
#include <TROOT.h>

#include "RooCategory.h"
#include "RooRandom.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace RooStats;
using namespace std;

// StandardHypoTestInvDemo: modified (ouput & some settings) standard rooStat macro for performing an inverted  hypothesis test for computing an interval from L.Moneta / K.Cramer

bool plotHypoTestResult = true;          // plot test statistic result at each point
bool writeResult = true;                 // write HypoTestInverterResult in a file 
TString resultFileName;                  // file with results (by default is built automatically using the workspace input file name)
bool optimize = true;                    // optmize evaluation of test statistic 
bool useVectorStore = true;              // convert data to use new roofit data store 
bool generateBinned = false;             // generate binned data sets 
bool noSystematics = false;              // force all systematics to be off (i.e. set all nuisance parameters as constat
										 // to their nominal values)
double nToysRatio = 2;                   // ratio Ntoys S+b/ntoysB
double maxPOI = -1;                      // max value used of POI (in case of auto scan) 
bool useProof = false;                    // use Proof Light when using toys (for freq or hybrid)
int nworkers = 21;                        // number of worker for Proof
bool rebuild = false;                    // re-do extra toys for computing expected limits and rebuild test stat
										 // distributions (N.B this requires much more CPU (factor is equivalent to nToyToRebuild)
int nToyToRebuild = 100;                 // number of toys used to rebuild 
int initialFit = -1;                     // do a first  fit to the model (-1 : default, 0 skip fit, 1 do always fit) 
int randomSeed = -1;                     // random seed (if = -1: use default value, if = 0 always random )
										 // NOTE: Proof uses automatically a random seed

int nAsimovBins = 0;                     // number of bins in observables used for Asimov data sets (0 is the default and it is given by workspace, typically is 100)

bool reuseAltToys = false;                // reuse same toys for alternate hypothesis (if set one gets more stable bands)

std::string massValue = "";              // extra string to tag output file of result 
std::string  minimizerType = "";                  // minimizer type (default is what is in ROOT::Math::MinimizerOptions::DefaultMinimizerType()
int   printLevel = 0;                    // print level for debugging PL test statistics and calculators  


HypoTestInvTool::LimitResults
StandardHypoTestInvDemo(int enne, TString esse, const char * infile = 0,
	const char * wsName = "combined",
	const char * modelSBName = "ModelConfig",
	const char * modelBName = "",
	const char * dataName = "obsData",
	int calculatorType = 0,
	int testStatType = 0,
	bool useCLs = true,
	int npoints = 6,
	double poimin = 0,
	double poimax = 5,
	int ntoys = 1000,
	bool useNumberCounting = false,
	const char * nuisPriorName = 0) {

	RooWorkspace::autoImportClassCode(kTRUE);
	/*

	  Other Parameter to pass in tutorial
	  apart from standard for filename, ws, modelconfig and data

	  type = 0 Freq calculator
	  type = 1 Hybrid calculator
	  type = 2 Asymptotic calculator
	  type = 3 Asymptotic calculator using nominal Asimov data sets (not using fitted parameter values but nominal ones)

	  testStatType = 0 LEP
	  = 1 Tevatron
	  = 2 Profile Likelihood
	  = 3 Profile Likelihood one sided (i.e. = 0 if mu < mu_hat)
	  = 4 Profiel Likelihood signed ( pll = -pll if mu < mu_hat)
	  = 5 Max Likelihood Estimate as test statistic
	  = 6 Number of observed event as test statistic

	  useCLs          scan for CLs (otherwise for CLs+b)

	  npoints:        number of points to scan , for autoscan set npoints = -1

	  poimin,poimax:  min/max value to scan in case of fixed scans
	  (if min >  max, try to find automatically)

	  ntoys:         number of toys to use

	  useNumberCounting:  set to true when using number counting events

	  nuisPriorName:   name of prior for the nnuisance. This is often expressed as constraint term in the global model
	  It is needed only when using the HybridCalculator (type=1)
	  If not given by default the prior pdf from ModelConfig is used.

	  extra options are available as global paramwters of the macro. They major ones are:

	  plotHypoTestResult   plot result of tests at each point (TS distributions) (defauly is true)
	  useProof             use Proof   (default is true)
	  writeResult          write result of scan (default is true)
	  rebuild              rebuild scan for expected limits (require extra toys) (default is false)
	  generateBinned       generate binned data sets for toys (default is false) - be careful not to activate with
	  a too large (>=3) number of observables
	  nToyRatio            ratio of S+B/B toys (default is 2)


	*/



	TString fileName(infile);
	if (fileName.IsNull()) {
		fileName = "results/example_combined_GaussExample_model.root";
		std::cout << "Use standard file generated with HistFactory : " << fileName << std::endl;
	}

	// open file and check if input file exists
	TFile * file = TFile::Open(fileName);

	// if input file was specified but not found, quit
	if (!file && !TString(infile).IsNull()) {
		cout << "file " << fileName << " not found" << endl;
		throw runtime_error("File not found in run limit");
	}

	// if default file not found, try to create it
	if (!file) {
		// Normally this would be run on the command line
		cout << "will run standard hist2workspace example" << endl;
		gROOT->ProcessLine(".! prepareHistFactory .");
		gROOT->ProcessLine(".! hist2workspace config/example.xml");
		cout << "\n\n---------------------" << endl;
		cout << "Done creating example input" << endl;
		cout << "---------------------\n\n" << endl;

		// now try to access the file again
		file = TFile::Open(fileName);

	}

	if (!file) {
		// if it is still not there, then we can't continue
		cout << "Not able to run hist2workspace to create example input" << endl;
		throw runtime_error("Couldn't get the file open for some weird reason.");
	}

	HypoTestInvTool calc;

	// set parameters
	calc.SetParameter("PlotHypoTestResult", plotHypoTestResult);
	calc.SetParameter("WriteResult", writeResult);
	calc.SetParameter("Optimize", optimize);
	calc.SetParameter("UseVectorStore", useVectorStore);
	calc.SetParameter("GenerateBinned", generateBinned);
	calc.SetParameter("NToysRatio", nToysRatio);
	calc.SetParameter("MaxPOI", maxPOI);
	calc.SetParameter("UseProof", useProof);
	calc.SetParameter("NWorkers", nworkers);
	calc.SetParameter("Rebuild", rebuild);
	calc.SetParameter("ReuseAltToys", reuseAltToys);
	calc.SetParameter("NToyToRebuild", nToyToRebuild);
	calc.SetParameter("MassValue", massValue.c_str());
	calc.SetParameter("MinimizerType", minimizerType.c_str());
	calc.SetParameter("PrintLevel", printLevel);
	calc.SetParameter("InitialFit", initialFit);
	calc.SetParameter("ResultFileName", resultFileName);
	calc.SetParameter("RandomSeed", randomSeed);
	calc.SetParameter("AsimovBins", nAsimovBins);
	calc.SetParameter("NoSystematics", noSystematics);


	RooWorkspace * w = dynamic_cast<RooWorkspace*>(file->Get(wsName));
	HypoTestInverterResult * r = 0;
	std::cout << w << "\t" << fileName << std::endl;
	if (w != NULL) {
		r = calc.RunInverter(enne, esse, w, modelSBName, modelBName,
			dataName, calculatorType, testStatType, useCLs,
			npoints, poimin, poimax,
			ntoys, useNumberCounting, nuisPriorName);
		if (!r) {
			std::cerr << "Error running the HypoTestInverter - Exit " << std::endl;
			throw runtime_error("Error running the HypnoTestInverter");
		}
	}
	else {
		// case workspace is not present look for the inverter result
		std::cout << "Reading an HypoTestInverterResult with name " << wsName << " from file " << fileName << std::endl;
		r = dynamic_cast<HypoTestInverterResult*>(file->Get(wsName)); //
		if (!r) {
			std::cerr << "File " << fileName << " does not contain a workspace or an HypoTestInverterResult - Exit "
				<< std::endl;
			file->ls();
			throw runtime_error("File doesn't contain a workspace.");
		}
	}

	auto expLimitSG = calc.AnalyzeResult(r, calculatorType, testStatType, useCLs, npoints, infile);

	return expLimitSG;
}



void ReadResult(const char * fileName, const char * resultName = "", bool useCLs = true) {
	// read a previous stored result from a file given the result name

	Int_t enne = -1;
	TString esse = "";
	StandardHypoTestInvDemo(enne, esse, fileName, resultName, "", "", "", 0, 0, useCLs);
}

// Get a value from a map of sys errors. Fail badly if it is missing.
double get_error(const map<string, double> &table, const string &key) {
	auto v = table.find(key);
	if (v == table.end()) {
		throw runtime_error("Unable to find systematic error " + key + ".");
	}
	return v->second;
}

/* Simultanous ABCD code  (S.giagu) */
/*
 * n[4] = {n_A, n_B, n_C, n_D} <-- number of observed events in regions A, B, C, D
 * s[4] = {s_A, s_B, s_C, s_D} <-- number of signal events in regions A, B, C, D
 * b[4] = {b_A, b_B, b_C, b_D} <-- number of BG events (estimated from MC) in regions A, B, C, D
 * c[4] = {c_A, c_B, c_C, c_D} <-- number of other BG events like cosmics etc.. (indipendently estimated from data) in regions A, B, C, D
 * useB = kFALSE <-- don't use BG events estimated from MC;  kTRUE <-- use them
 * useC = kFALSE <-- don't use other BG events (like cosmics etc..) indipendently estimated from data;  kTRUE <-- use them
 * blindA: kTRUE <-- keep signal region blind (i.e. test done assuming n_A = ABCD_exp_A),  kFALSE <-- use obs events in signal region
 *
 */
HypoTestInvTool::LimitResults simultaneousABCD(const Double_t n[4], const Double_t s[4], const Double_t b[4], const Double_t c[4],
	TString out_filename,
	Bool_t useB, // Use background as estimated in MC
	Bool_t useC, // Use other background events (do subtraction of c above
	Bool_t blindA, // Assume no signal, so we get expected limits
	Int_t calculationType, // See comments below - 0 for toys, 2 for asym fit
	Int_t par_ntoys, // number of events in Asimov sample in case of calc type 2 or 3, number of events in each toys for type 0; default should be : 50000
	map<string,double> systematic_errors // The errors to be used in the fit
)
{

	// set RooFit random seed to a fix value for reproducible results
	RooRandom::randomGenerator()->SetSeed(4357);

	// init
	RooWorkspace::autoImportClassCode(kTRUE); // set default behaviour of RooWorkspace when importing new classes

	//Inputs
	// signal
	Double_t ns_A = 41;//s[0];
	if (ns_A <= 0) {
	        std::cout << "ERROR: 0 signal events in signal region (A)!!! --> Check inputs!  s[0] = " << ns_A << std::endl;
		throw runtime_error("No signal events found in signal region!");
	}
	Double_t ns_B = 3418;//s[1];
	Double_t ns_C = 931;//s[2];
	Double_t ns_D = 105;//s[3];
	// data
	Double_t nd_A = n[0];
	Double_t nd_B = n[1];
	Double_t nd_C = n[2];
	Double_t nd_D = n[3];
	// MC based BG
	Double_t nb_A = b[0];
	Double_t nb_B = b[1];
	Double_t nb_C = b[2];
	Double_t nb_D = b[3];
	// Independently DATA based BG
	Double_t nc_A = c[0];
	Double_t nc_B = c[1];
	Double_t nc_C = c[2];
	Double_t nc_D = c[3];

	// Some initial printout ...
	Double_t nd_A_expected = 0.0;
	if (nd_C > 0) nd_A_expected = nd_B * nd_D / nd_C;
	std::cout << "Obs events in signal region (A) estimated from control regions using PLAIN ABCD: " << nd_A_expected << std::endl;
	std::cout << "              " << std::endl;

	if (blindA) { //don't use observed data in signal region (for expected yields) but expectation from PLAIN ABCD
		nd_A = nd_A_expected;
	}

	std::cout << "Input yields with signal region " << (blindA ? "blinded: " : "unblinded: ") << std::endl;
	std::cout << "Observed A/B/C/D: " << nd_A << " / " << nd_B << " / " << nd_C << " / " << nd_D << std::endl;
	std::cout << "Signal   A/B/C/D: " << ns_A << " / " << ns_B << " / " << ns_C << " / " << ns_D << std::endl;
	if (useB)
		std::cout << "MC BG A/B/C/D:    " << nb_A << " / " << nb_B << " / " << nb_C << " / " << nb_D << std::endl;
	if (useC)
		std::cout << "Data BG A/B/C/D:  " << nc_A << " / " << nc_B << " / " << nc_C << " / " << nc_D << std::endl;
	std::cout << "              " << std::endl;

	// guess some initial values (to speedup fit convergence) for the parameters ...
	Double_t sr_B = ns_B / ns_A;
	Double_t sr_C = ns_C / ns_A;
	Double_t sr_D = ns_D / ns_A;
	Double_t mu_guess = 3.0 / ns_A; // starting guess for mu = N_sig_UL / N_sig_exp assume no signal observed (~3 events UL at 95% CL)

	std::cout << "Signal ratios B/A C/A and D/A: " << sr_B << " / " << sr_C << " / " << sr_D << std::endl;
	std::cout << "Guess mu: " << mu_guess << std::endl;
	std::cout << std::endl;

	Double_t nq_A_guess = nd_A_expected; // starting guess from ABCD events in signal region (from ABCD ansatz + other BG)
	if (useB) nq_A_guess -= nb_A;
	if (useC) nq_A_guess -= nc_A;
	if (nq_A_guess < 0) nq_A_guess = 0.0;
	Double_t nq_B_guess = nd_B;
	Double_t nq_D_guess = nd_D;
	std::cout << "Guess Multijet BG A/B/D: " << nq_A_guess << " / " << nq_B_guess << " / " << nq_D_guess << std::endl;
	std::cout << "              " << std::endl;


	if (nq_A_guess <= 0) {
		nq_A_guess = 3.0;
		std::cout << "WARNING: zero events for nq_A_guess, used 3 events" << std::endl;
	}
	Double_t ta_A = nq_A_guess; // tau parameters guesses (see Likelihood ABCD note in stat forum for definition)

	Double_t ta_B = 3.0;
	Double_t ta_D = 3.0;
	if (nq_B_guess > 0)
		ta_B = nq_B_guess / nq_A_guess;
	else
		std::cout << "WARNING: zero events for nq_B_guess, used 3 events" << std::endl;
	if (nq_D_guess > 0)
		ta_D = nq_D_guess / nq_A_guess;
	else
		std::cout << "WARNING: zero events for nq_D_guess, used 3 events" << std::endl;

	std::cout << "tau Multijet A/B/D: " << ta_A << " / " << ta_B << " / " << ta_D << std::endl;
	std::cout << "              " << std::endl;


	// make model
	RooWorkspace* wspace = new RooWorkspace("wspace", "ABCD workspace");
	wspace->addClassDeclImportDir("."); // add code import paths
	wspace->addClassImplImportDir(".");

	// observed events
	wspace->factory("NA[0,20000]");
	wspace->factory("NB[0,20000]");
	wspace->factory("NC[0,20000]");
	wspace->factory("ND[0,20000]");

	// POI
	wspace->factory(TString::Format("mu[%f,0,1]", mu_guess));  // mu = NsA/Ns0 (Ns0 = expected events)
	//note: SM means mu=0 (used for the BG only hypotesis for the expected limit

	// pdf parameters
	wspace->factory(TString::Format("lumi[%f]", 1.0));    // Luminosity (scale factor wrt the luinosity on dat)

	wspace->factory(TString::Format("Ns0[%f,0,20000]", 41)); //Expected signal in region A
	wspace->factory(TString::Format("effB[%f,0,5000]", sr_B)); //Sig. eff. in region B wrt region A
	wspace->factory(TString::Format("effC[%f,0,5000]", sr_C)); //Sig. eff. in region C wrt region A
	wspace->factory(TString::Format("effD[%f,0,5000]", sr_D)); //Sig. eff. in region D wrt region A

	wspace->factory(TString::Format("Nq[%f,0,20000]", 23.5)); //number of Multijet events in region A
	wspace->factory(TString::Format("tauB[%f,0,1000]", ta_B)); //Multijet BG eff. in region B wrt region A
	wspace->factory(TString::Format("tauD[%f,0,1000]", ta_D)); //Multijet BG eff. in region D wrt region A

	if (useB) {
		wspace->factory(TString::Format("NbA[%f,0,1000]", nb_A)); //number of MC BG events in region A
		wspace->factory(TString::Format("NbB[%f,0,1000]", nb_B)); // in region B
		wspace->factory(TString::Format("NbC[%f,0,1000]", nb_C)); // C
		wspace->factory(TString::Format("NbD[%f,0,1000]", nb_D)); // ... and D
	}

	if (useC) {
		wspace->factory(TString::Format("NcA[%f,0,1000]", nc_A)); //number of Other data-driven BG events in region A
		wspace->factory(TString::Format("NcB[%f,0,1000]", nc_B)); // in region B
		wspace->factory(TString::Format("NcC[%f,0,1000]", nc_C)); // C
		wspace->factory(TString::Format("NcD[%f,0,1000]", nc_D)); // ... and D
	}

	std::cout << ">>>>> Lumi error = " << get_error (systematic_errors, "lumi") << std::endl;
	std::cout << ">>>>>   MC error = " << get_error (systematic_errors, "mc_eff") << std::endl;
	std::cout << ">>>>> ABCD error = " << get_error (systematic_errors, "abcd") << std::endl;

	//Systematic uncertanties' nuisance parameters 
	//lumi
	wspace->factory("alpha_lumi[1, 0, 10]");
	wspace->factory("nom_lumi[1, 0, 10]");
	ostringstream lumi_error;
	lumi_error << "nom_sigma_lumi[" << get_error (systematic_errors, "lumi") << "]";
	wspace->factory(lumi_error.str().c_str());  // <--  2.1% final run2 2015
	wspace->factory("Gaussian::constraint_lumi(nom_lumi, alpha_lumi, nom_sigma_lumi)");

	wspace->factory("alpha_S[1, 0, 2]");  //systematic nuisance on signal (efficiencies etc.) and on MC bg
	wspace->factory("nom_S[1, 0, 10]");
	ostringstream mc_events_error;
	mc_events_error << "nom_sigma_S[" << "0.17" << "]";//get_error(systematic_errors, "mc_eff") << "]";
	wspace->factory(mc_events_error.str().c_str()); // 24% totale displaced LJ analysis 2016
	wspace->factory("Gaussian::constraint_S(nom_S, alpha_S, nom_sigma_S)");

	wspace->factory("alpha_Q[1, 0, 2]");  //systematic nuisance on Multijet   
	wspace->factory("nom_Q[1, 0, 5]");
	ostringstream abcd_error;
	abcd_error << "nom_sigma_Q[" << "0.34" << "]";//get_error(systematic_errors, "abcd") << "]";
	wspace->factory(abcd_error.str().c_str());   //30%% on QCD from ABCD variations and closure tests
	wspace->factory("Gaussian::constraint_Q(nom_Q, alpha_Q, nom_sigma_Q)");

	if (useC) {
		wspace->factory("alpha_C[1, 0, 10]");  //systematic nuisance on other data-driven BG
		wspace->factory("nom_C[1, 0, 10]");
		wspace->factory("nom_sigma_C[0.32]");   //20% on cosmic BG (just as an example)
		wspace->factory("Gaussian::constraint_C(nom_C, alpha_C, nom_sigma_C)");
	}

	if (useB) {
		wspace->factory("alpha_B[1, 0, 10]");  //systematic nuisance on MC BG
		wspace->factory("nom_B[1, 0, 10]");
		wspace->factory("nom_sigma_B[0.32]");   //10% on MC BG (just as an example)
		wspace->factory("Gaussian::constraint_B(nom_B, alpha_B, nom_sigma_B)");
	}

	// PDF
	wspace->factory("prod::NsA(mu,Ns0,lumi,alpha_lumi,alpha_S)");      // expected signal events in A: mu*Ns0*L
	wspace->factory("prod::NsB(mu,Ns0,effB,lumi,alpha_lumi,alpha_S)"); // expected signal events in B: mu*Ns0*L*effB
	wspace->factory("prod::NsC(mu,Ns0,effC,lumi,alpha_lumi,alpha_S)");
	wspace->factory("prod::NsD(mu,Ns0,effD,lumi,alpha_lumi,alpha_S)");

	wspace->factory("prod::NbQA(Nq,lumi, alpha_Q)");    // expected Multijet (i.e. ABCD) BG events in A: Nq*SysQ
	wspace->factory("prod::NbQB(Nq,lumi, tauB)");       // expected Multijet BG events in B: Nq*tauB;
	wspace->factory("prod::NbQC(Nq,lumi, tauB,tauD)");  // ABCD ansatz: expected Multijet BG events in C: Nq*tauB*tauD;
	wspace->factory("prod::NbQD(Nq,lumi, tauD)");       // expected Multijet BG events in D: Nq*tauD;

	if (useB) {
		wspace->factory("prod::NbBA(NbA,lumi,alpha_lumi,alpha_B)");   // expected MC BG events in region A: NbA*sysB*sysLumi (MC)
		wspace->factory("prod::NbBB(NbB,lumi,alpha_lumi,alpha_B)");
		wspace->factory("prod::NbBC(NbC,lumi,alpha_lumi,alpha_B)");
		wspace->factory("prod::NbBD(NbD,lumi,alpha_lumi,alpha_B)");
	}

	if (useC) {
		wspace->factory("prod::NbCA(NcA,lumi, alpha_C)");   // expected cosm. BG events in A: NcA*sysCos 
		wspace->factory("prod::NbCB(NcB,lumi, alpha_C)");
		wspace->factory("prod::NbCC(NcC,lumi, alpha_C)");
		wspace->factory("prod::NbCD(NcD,lumi, alpha_C)");
	}

	if (useC && useB) {
		wspace->factory("sum::NexpA(NsA,NbQA,NbBA,NbCA)");
		wspace->factory("sum::NexpB(NsB,NbQB,NbBB,NbCB)");
		wspace->factory("sum::NexpC(NsC,NbQC,NbBC,NbCC)");
		wspace->factory("sum::NexpD(NsD,NbQD,NbBD,NbCD)");
	}
	else if (useC && !useB) {
		wspace->factory("sum::NexpA(NsA,NbQA,NbCA)");
		wspace->factory("sum::NexpB(NsB,NbQB,NbCB)");
		wspace->factory("sum::NexpC(NsC,NbQC,NbCC)");
		wspace->factory("sum::NexpD(NsD,NbQD,NbCD)");
	}
	else if (!useC && useB) {
		wspace->factory("sum::NexpA(NsA,NbQA,NbBA)");
		wspace->factory("sum::NexpB(NsB,NbQB,NbBB)");
		wspace->factory("sum::NexpC(NsC,NbQC,NbBC)");
		wspace->factory("sum::NexpD(NsD,NbQD,NbBD)");
	}
	else {
		wspace->factory("sum::NexpA(NsA,NbQA)");
		wspace->factory("sum::NexpB(NsB,NbQB)");
		wspace->factory("sum::NexpC(NsC,NbQC)");
		wspace->factory("sum::NexpD(NsD,NbQD)");
	}

	wspace->factory("Poisson::obsA(NA,NexpA)");
	wspace->factory("Poisson::obsB(NB,NexpB)");
	wspace->factory("Poisson::obsC(NC,NexpC)");
	wspace->factory("Poisson::obsD(ND,NexpD)");


	if (useC && useB) {
		wspace->factory("PROD::model(obsA,obsB,obsC,obsD,constraint_lumi,constraint_Q,constraint_S, constraint_C, constraint_B)");
	}
	else if (useC && !useB) {
		wspace->factory("PROD::model(obsA,obsB,obsC,obsD,constraint_lumi,constraint_Q,constraint_S, constraint_C)");
	}
	else if (!useC && useB) {
		wspace->factory("PROD::model(obsA,obsB,obsC,obsD,constraint_lumi,constraint_Q,constraint_S, constraint_B)");
	}
	else {
		wspace->factory("PROD::model(obsA,constraint_lumi,constraint_Q,constraint_S)");
	}

	// sets
	TString the_poi = "mu";
	TString the_nuis = ",tauB,tauD,alpha_lumi,alpha_Q,alpha_S";
	TString the_glob = ",nom_lumi,nom_Q,nom_S";
	if (useC) {
		the_nuis += ",alpha_C";
		the_glob += ",nom_C";
	}
	if (useB) {
		the_nuis += ",alpha_B";
		the_glob += ",nom_B";
	}
	std::cout << "poi:        " << the_poi << std::endl;
	std::cout << "nuisances:  " << the_nuis << std::endl;
	std::cout << "global var: " << the_glob << std::endl;

	wspace->defineSet("obs", "NA,NB,NC,ND");
	wspace->defineSet("poi", the_poi);
	wspace->defineSet("nuis", the_nuis);
	wspace->defineSet("glob", the_glob);

	//fix needed parameters

	TString    interesting = "," + the_poi + "," + the_nuis; // note leading comma

	TIterator *itr;
	itr = wspace->pdf("model")->getParameters(*wspace->set("obs"))->createIterator();
	TObject *obj(0);
	RooRealVar *rrv(0);
	RooCategory *rc(0);
	while ((obj = itr->Next())) {
		if (interesting.Contains(TString::Format(",%s", obj->GetName())) == kFALSE) {
			// try RooRealVar
			rrv = dynamic_cast<RooRealVar*>(obj);
			rc = dynamic_cast<RooCategory*>(obj);

			if (rrv) {
				rrv->setConstant(kTRUE);
				std::cout << "setting " << rrv->GetName() << " const(val = " << rrv->getVal() << ")" << std::endl;
			}
			else if (rc) {
				rc->setConstant(kTRUE);
				std::cout << "setting " << rc->GetName() << " const(val = " << rc->getLabel() << ")" << std::endl;
			}
			else
				std::cout << "unable to set " << obj->GetName() << " const(not RRV)" << std::endl;
		}
	} // loop on pdf parameters

	// inspect workspace
	wspace->Print("V");

	////////////////////////////////////////////////////////////
	// Generate toy data
	// generate toy data assuming current value of the parameters
	// add Verbose() to see how it's being generated
	//RooDataSet* data = wspace->pdf("model")->generate(*wspace->set("obs"),1);
	//data->Print("v");
	// or input real data
	// add Verbose() to see how it's being generated
	RooDataSet* data = new RooDataSet("data", "obsData", *wspace->set("obs"));
	wspace->var("NA")->setVal(nd_A);
	wspace->var("NB")->setVal(nd_B);
	wspace->var("NC")->setVal(nd_C);
	wspace->var("ND")->setVal(nd_D);
	data->add(*wspace->set("obs"));
	data->Print("v");

	// import into workspace.
	wspace->import(*data, RooFit::Rename("obsData"));

	/////////////////////////////////////////////////////
	// Now the statistical tests
	// model config
	ModelConfig* mc = new ModelConfig("mc");
	mc->SetWorkspace(*wspace);
	mc->SetPdf(*wspace->pdf("model"));
	mc->SetObservables(*wspace->set("obs"));
	mc->SetParametersOfInterest(*wspace->set("poi"));
	mc->SetNuisanceParameters(*wspace->set("nuis"));
	mc->SetGlobalObservables(RooArgSet());
	wspace->import(*mc);

	wspace->Print();

	std::cout << "Writing on " << out_filename << std::endl;
	wspace->writeToFile(out_filename);
	std::cout << "All OK." << std::endl;

	// CLs test

	// calculationType
	// type = 0 Freq calculator   (for toys)
	// type = 1 Hybrid calculator (don't use this)
	// type = 2 Asymptotic calculator  (for asymptotic approximation)
	// type = 3 Asymptotic calculator using nominal Asimov data sets (not using fitted parameter values but nominal ones)
	//
	Int_t testStatType = 3;
	// testStatType = 0 LEP
	//              = 1 Tevatron 
	//              = 2 Profile Likelihood two sided
	//              = 3 Profile Likelihood one sided (i.e. = 0 if mu < mu_hat)  <-- for Upper Limits (use this)
	//              = 4 Profile Likelihood signed ( pll = -pll if mu < mu_hat) 
	//              = 5 Max Likelihood Estimate as test statistic
	//              = 6 Number of observed event as test statistic

	Double_t par_poi_min = 0.0;   // mu scanned from par_poi_min to par_poi_max with par_npointscan steps
	Double_t par_poi_max = 1.;//0.01;
	Int_t    par_npointscan = 500; // default: 100

	auto score = StandardHypoTestInvDemo(0, "", out_filename, "wspace", "mc", "mc", "obsData", calculationType, testStatType, true, par_npointscan, par_poi_min, par_poi_max, par_ntoys);

	return score;
}
