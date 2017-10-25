#include <cmath>

// Sadly, M_PI is not part of the standard

// double M_PI = 3.14159265358979323846;

void CalRSelection(){
  return;
}

// Stand alone file to encode the specific cuts that are used in the CalR analysis.
// Putting them here makes it easy to use them in multiple analyses, and also
// keeps them clean and in one place.

// For the analysis selection, there are some implied cuts:
// Trigger Selection: event_passCalRatio_TAU60_noiso
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
  bool event_selection(
		       double j1_pt, double j2_pt,
		       double j1_eta, double j2_eta,
		       bool j1_isGoodLLP, bool j2_isGoodLLP,
		       double j1_phi, double j2_phi,
		       double j1_time, double j2_time,
		       double event_HTMiss, double event_HT,
		       double sumMinDRTrk2pt50, double j1_BDTscore, double j2_BDTscore
		       )
  {
        auto r = j1_pt > 150.0 && j2_pt > 120.0
	    && abs(j1_eta) < 2.5 && abs(j2_eta) < 2.5
            && j1_isGoodLLP && j2_isGoodLLP
	    && abs(wrapPhi(j1_phi - j2_phi)) > 0.75
            && j1_time > -3.0 && j2_time > -3.0
            && j1_time < 15.0 && j2_time < 15.0
            && event_HTMiss / event_HT < 0.3
            && sumMinDRTrk2pt50 > 0.5 
            && j1_BDTscore > 0.2 && j2_BDTscore > -0.2;
        //std::cout << r 
        //    << " - j1pt: " << j1_pt << " j2pt:" << j2_pt 
        //    << " - eta: " << j1_eta << ", " << j2_eta
        //    << " - isgood: " << j1_isGoodLLP << ", " << j2_isGoodLLP
        //    << " - time: " << j1_time << ", " << j2_time
        //    << " - htratio: " << event_HTMiss / event_HT
        //    << " - sum DR" << sumMinDRTrk2pt50
        //    << std::endl;
        return r;
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
  int event_ABCD_plane(double j1_pt, double j2_pt,
		       double j1_bdt13lxy, double j2_bdt13lxy,
		       double sumMinDRTrk2pt50)
  {
    // Is it on the ABCD plane at all?
        bool inRegion = j1_pt > 150.0 && j2_pt > 120.0
            && j1_bdt13lxy > 0.2
	  && j2_bdt13lxy > -0.2;
        if (!inRegion) {
	  return 0;
        }

        // Calc the axes that will tell us where this thing is.
        auto sumBDT13Lxy = j1_bdt13lxy + j2_bdt13lxy;

        auto sumMinDRUpper = sumMinDRTrk2pt50 > 1.5;
        auto sumBDT13LxyUpper = sumBDT13Lxy > 0.15;

        if (sumMinDRUpper && sumBDT13LxyUpper) {
	  return 1; // A
        }
        if (!sumMinDRUpper && sumBDT13LxyUpper) {
	  return 2; // B
        }
        if (sumMinDRUpper && !sumBDT13LxyUpper) {
	  return 3; // C
        }

        return 4; // D
  }



