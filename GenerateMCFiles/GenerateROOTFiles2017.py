#!/usr/bin/env python
import ROOT as r
import argparse
import os
from array import array
r.gROOT.SetBatch(1)

#Import all functions from the selection macro 
r.gSystem.Load('CalRSelection2017_C.so')

parser = argparse.ArgumentParser(description='Slim MC file to use in extrapolation code.')
#parser.add_argument('-s', metavar='sample', dest='samplename', nargs=1, help='Sample to process, e.g. mH600_mS150_lt5m')

parser.add_argument('-s', '--sample', help='Sample to process, e.g. mH600_mS150_lt5m.root')
parser.add_argument('-o', '--outfile', help='Name of output root file')
parser.add_argument('-n', '--nevents', help='Number of events to process (default -1)', default='-1', type=int)
parser.add_argument('-m', '--selection', help='Selection to use, 1 or 2', type=int)
args = parser.parse_args()
parser.print_help()

#Open the right file
treeName = 'recoTree'
inFile = r.TFile(args.sample)
tree = inFile.Get(treeName)

#Define the output file and tree
f = r.TFile( args.outfile, 'recreate' ) 
t = r.TTree( 'extrapTree', 'Used as input for the extrapolation' ) 

#Set up the branches in the output tree
eventNumber = array( 'i', [ 0 ] )
PassedCalRatio = array( 'i', [ 0 ] )

llp1_pt = array( 'd', [ 0. ] )
llp2_pt = array( 'd', [ 0. ] )
llp1_eta = array( 'd', [ 0. ] )
llp2_eta = array( 'd', [ 0. ] )
llp1_phi = array( 'd', [ 0. ] )
llp2_phi = array( 'd', [ 0. ] )
llp1_E = array( 'd', [ 0. ] )
llp2_E = array( 'd', [ 0. ] )
llp1_Lxy = array( 'd', [ 0. ] )
llp2_Lxy = array( 'd', [ 0. ] )

event_weight = array( 'd', [ 0. ] )

RegionA = array( 'i', [ 0 ] )
RegionB = array( 'i', [ 0 ] )
RegionC = array( 'i', [ 0 ] )
RegionD = array( 'i', [ 0 ] )

t.Branch( 'eventNumber', eventNumber, 'eventNumber/I')
t.Branch( 'PassedCalRatio', PassedCalRatio, 'PassedCalRatio/I')

t.Branch( 'llp1_pt', llp1_pt, 'llp1_pt/D')
t.Branch( 'llp2_pt', llp2_pt, 'llp2_pt/D')
t.Branch( 'llp1_eta', llp1_eta, 'llp1_eta/D')
t.Branch( 'llp2_eta', llp2_eta, 'llp2_eta/D')
t.Branch( 'llp1_phi', llp1_phi, 'llp1_phi/D')
t.Branch( 'llp2_phi', llp2_phi, 'llp2_phi/D')
t.Branch( 'llp1_E', llp1_E, 'llp1_E/D')
t.Branch( 'llp2_E', llp2_E, 'llp2_E/D')
t.Branch( 'llp1_Lxy', llp1_Lxy, 'llp1_Lxy/D')
t.Branch( 'llp2_Lxy', llp2_Lxy, 'llp2_Lxy/D')

t.Branch( 'event_weight', event_weight, 'event_weight/D')

t.Branch( 'RegionA', RegionA, 'RegionA/I')
t.Branch( 'RegionB', RegionB, 'RegionB/I')
t.Branch( 'RegionC', RegionC, 'RegionC/I')
t.Branch( 'RegionD', RegionD, 'RegionD/I')

#Keep track of the number of events
counter = 0
maxcount = args.nevents

n_preselection = 0
n_eventBDT = 0
n_triggerMatch = 0
n_timing = 0
n_BIBveto = 0
n_MHToHT = 0
n_logRatio = 0
n_pT_sel1 = 0
n_pT_sel2 = 0
n_RegionA_sel1 = 0
n_RegionA_sel2 = 0

#For each event:
# - check if it passes the trigger and selection
# - where it lies in the ABCD plane
# - write out to new file
for ev in tree:
  
  counter += 1
  if maxcount > 0:
    if counter > maxcount: break

  isSelected = False
  PassBibJetVeto=True

  for i in range (ev.CalibJet_BDT3weights_bib.size()):
    if (ev.CalibJet_BDT3weights_bib[i] > 0.6 and ev.CalibJet_pT[i] > 40 and abs(ev.CalibJet_eta[i]) < 2.5 and ev.CalibJet_isGoodLLP[i] ): PassBibJetVeto = False

  if(ev.BDT3weights_signal_cleanJet_index.size()>=2 and ev.event_sumMinDR > 0.5 and ev.event_passCalRatio_cleanLLP_TAU60 and r.readVecBool(ev.CalibJet_isGoodLLP,ev.BDT3weights_signal_cleanJet_index[0]) and r.readVecBool(ev.CalibJet_isGoodLLP,ev.BDT3weights_signal_cleanJet_index[1])): 
    n_preselection+=1
    if(ev.eventBDT_value > 0.05):
      n_eventBDT+=1
      # ev.CalibJet_logRatio[ev.BDT3weights_signal_cleanJet_index[0]] > 1.2 or same for [1]
      if((r.readVecBool(ev.CalibJet_isCRHLTJet,ev.BDT3weights_signal_cleanJet_index[0]) and ev.CalibJet_minDRTrkpt2[ev.BDT3weights_signal_cleanJet_index[0]] > 0.2 and ev.CalibJet_logRatio[ev.BDT3weights_signal_cleanJet_index[0]] > 1.2) or (r.readVecBool(ev.CalibJet_isCRHLTJet,ev.BDT3weights_signal_cleanJet_index[1]) and ev.CalibJet_minDRTrkpt2[ev.BDT3weights_signal_cleanJet_index[1]] >0.2 and ev.CalibJet_logRatio[ev.BDT3weights_signal_cleanJet_index[1]] > 1.2)):
        n_triggerMatch+=1
        if(ev.CalibJet_time[ev.BDT3weights_signal_cleanJet_index[0]] > -3 and ev.CalibJet_time[ev.BDT3weights_signal_cleanJet_index[1]] > -3 and ev.CalibJet_time[ev.BDT3weights_signal_cleanJet_index[0]] <15 and ev.CalibJet_time[ev.BDT3weights_signal_cleanJet_index[1]] < 15 and ev.CalibJet_time[ev.BDT3weights_bib_cleanJet_index[0]] > -3 and ev.CalibJet_time[ev.BDT3weights_bib_cleanJet_index[1]] > -3 and ev.CalibJet_time[ev.BDT3weights_bib_cleanJet_index[0]] < 15 and ev.CalibJet_time[ev.BDT3weights_bib_cleanJet_index[1]] < 15):
          n_timing+=1
          if(PassBibJetVeto):
            n_BIBveto+=1
            if(ev.event_MHToHT < 0.8):
              n_MHToHT+=1
              if((ev.CalibJet_logRatio[ev.BDT3weights_signal_cleanJet_index[0]] + ev.CalibJet_logRatio[ev.BDT3weights_signal_cleanJet_index[1]]) > 2):
                n_logRatio+=1
                if(ev.CalibJet_pT[ev.BDT3weights_signal_cleanJet_index[0]] > 100):
                  n_pT_sel1+=1
                  if(args.selection==1): isSelected = True
                  if(ev.event_sumMinDR>1.5 and ev.eventBDT_value > 0.1):
                    n_RegionA_sel1+=1
                if(ev.CalibJet_pT[ev.BDT3weights_signal_cleanJet_index[0]] > 160):
                  n_pT_sel2+=1
                  if(args.selection==2): isSelected = True
                  if(ev.event_sumMinDR>1.5 and ev.eventBDT_value > 0.1):
                    n_RegionA_sel2+=1
  
  passedTrigger = False
  passedTrigger = ev.event_passCalRatio_cleanLLP_TAU60
  
  region = 0
  region = r.event_ABCD_plane(ev.eventBDT_value,ev.event_sumMinDR)

  llp1_E[0] = ev.LLP_E[0]
  llp2_E[0] = ev.LLP_E[1]
  llp1_eta[0] = ev.LLP_eta[0]
  llp2_eta[0] = ev.LLP_eta[1]
  llp1_pt[0] = ev.LLP_pT[0]
  llp2_pt[0] = ev.LLP_pT[1]
  llp1_phi[0] = ev.LLP_phi[0]
  llp2_phi[0] = ev.LLP_phi[1]
  llp1_Lxy[0] = ev.LLP_Lxy[0]
  llp2_Lxy[0] = ev.LLP_Lxy[1]

  PassedCalRatio[0] = passedTrigger
  eventNumber[0] = ev.eventNumber
    
  event_weight[0] = ev.eventWeight * abs(ev.pileupEventWeight)

  RegionA[0],RegionB[0],RegionC[0],RegionD[0] = 0,0,0,0
  if (region == 1 and passedTrigger and isSelected): RegionA[0] = 1
  if (region == 2 and passedTrigger and isSelected): RegionB[0] = 1
  if (region == 3 and passedTrigger and isSelected): RegionC[0] = 1
  if (region == 4 and passedTrigger and isSelected): RegionD[0] = 1
#  print RegionA,RegionB,RegionC,RegionD

  t.Fill()


f.Write()
f.Close()

print "Cutflow results for sample ", args.sample
print "================================================="
print "                Preselection : ", n_preselection
print "             eventBDT > 0.05 : ", n_eventBDT
print "            Trigger matching : ", n_triggerMatch
print "     -3 < time(sig,bib) < 15 : ", n_timing
print "                  0 BIB jets : ", n_BIBveto
print "             HTmiss/HT < 0.8 : ", n_MHToHT
print "sum(logRatio(jet1,jet2)) > 2 : ", n_logRatio
print "-------------------------------------------------"
print "       Selection 1  pT > 100 : ", n_pT_sel1
print "                    Region A : ", n_RegionA_sel1
print "-------------------------------------------------"
print "       Selection 2  pT > 160 : ", n_pT_sel2
print "                    Region A : ", n_RegionA_sel2
