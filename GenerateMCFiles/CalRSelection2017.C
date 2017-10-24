#include <cmath>

// Sadly, M_PI is not part of the standard

// double M_PI = 3.14159265358979323846;

void CalRSelection2017(){
  return;
}

// Stand alone file to encode the specific cuts that are used in the CalR analysis.
// Putting them here makes it easy to use them in multiple analyses, and also
// keeps them clean and in one place.

// For the analysis selection, there are some implied cuts:
// Trigger Selection: event_passCalRatio_cleanLLP_TAU60_noiso
// At least two jets. The jets must be ordered in decreasing BDT (so j1 has the
// highest BDT value, and j2 has the second highest bdt value).

//helper function
double wrapPhi(double phi) {
  static const double M_2PI = 2 * M_PI;
  while (phi > M_PI) phi -= M_2PI;
  while (phi < -M_PI) phi += M_2PI;
  return phi;
}

bool readVecBool(std::vector<bool> vec, int index){
  for (size_t i =0; i < vec.size(); i++){
    if (i==index) return vec[index];
  }
}


  // This is basic selection. All events must pass this in order to be part of the analysis.
  // UPDATE Oct 2017: This selection is implemented in the GenerateROOTFiles2017.py script - it may not be up to date here!!
bool event_selection(int selection,
		     double j1s_pt,
		     bool j1s_isGoodLLP, bool j2s_isGoodLLP,
		     double j1s_time, double j2s_time,
		     double j1b_time, double j2b_time,
		     double event_MHToHT, double event_sumMinDR,
		     double j1s_minDRTrk2pt, double j2s_minDRTrk2pt,
		     bool j1s_isCRHLTJet, bool j2s_isCRHLTJet, 
		     double j1s_logRatio, double j2s_logRatio,
		     double eventBDT_value
		       )
  {
    if(selection == 1){
      auto r = ( (j1s_isCRHLTJet && j1s_logRatio > 1.2 && j1s_minDRTrk2pt > 0.2 )
      || (j2s_isCRHLTJet && j2s_logRatio > 1.2 && j2s_minDRTrk2pt > 0.2) ) 
      && j1s_time > -3 && j1s_time < 15 
      && j2s_time > -3 && j2s_time < 15
      && j1b_time > -3 && j1b_time < 15
      && j2b_time > -3 && j2b_time < 15
      && event_MHToHT < 0.8
      && j1s_pt > 100
      && (j1s_logRatio + j2s_logRatio) > 2
      && event_sumMinDR > 0.5
      && eventBDT_value > 0.1;
      
      return r;
    }
    if(selection == 2){
      auto r = ( (j1s_isCRHLTJet && j1s_logRatio > 1.2 && j1s_minDRTrk2pt > 0.2 )
      || (j2s_isCRHLTJet && j2s_logRatio > 1.2 && j2s_minDRTrk2pt > 0.2) ) 
      && j1s_time > -3 && j1s_time < 15 
      && j2s_time > -3 && j2s_time < 15
      && j1b_time > -3 && j1b_time < 15
      && j2b_time > -3 && j2b_time < 15
      && event_MHToHT < 0.8
      && j1s_pt > 160
      && (j1s_logRatio + j2s_logRatio) > 2
      && event_sumMinDR > 0.5
      && eventBDT_value > 0.1;

      return r; // DON'T YOU, FORGET ABOUT ME
    }
  }



  // This is the selection required to get into the ABCD plane, anywhere. But somewhere. :-)
  // As above the jets are BDT13Lxy ordered, not pT ordered!
  // 
  // esumMinDRTrk2pt50
  //    This is the sum of the CalibJet_minDRTrkpt2 TTree variable for all jets with |eta|<2.5 and
  //  with pT>50.0 GeV. No other requirements are made on the jets.
  //
  // Return value
  //        0        Reject event
  //        1        A
  //        2        B
  //        3        C
  //        4        D
  //
int event_ABCD_plane(double eventBDT_value,
		       double sumMinDRTrk2pt50)
  {

        // Calc the axes that will tell us where this thing is.
        auto sumMinDRUpper = sumMinDRTrk2pt50 > 1.5;
        auto eventBDTUpper = eventBDT_value > 0.1;

        if (sumMinDRUpper && eventBDTUpper) {
	  return 1; // A
        }
        if (!sumMinDRUpper && eventBDTUpper) {
	  return 2; // B
        }
        if (sumMinDRUpper && !eventBDTUpper) {
	  return 3; // C
        }

        return 4; // D
  }



