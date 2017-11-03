#!/usr/bin/env python
import ROOT as r
import argparse
import os
from array import array

r.gROOT.LoadMacro('atlasstyle-00-04-02/AtlasStyle.C')
r.gROOT.LoadMacro('atlasstyle-00-04-02/AtlasUtils.C')
r.gROOT.LoadMacro('atlasstyle-00-04-02/AtlasLabels.C')
r.SetAtlasStyle()
r.gROOT.SetBatch(1)
r.gStyle.SetOptLogy(1)
r.gStyle.SetOptLogx(1)

#Parse command line
parser = argparse.ArgumentParser(description='Compare 2017 limit to 2015 limit')
parser.add_argument('-e', '--extrapFile2017', help='Path to root file containing limit calculation results from 2017')
parser.add_argument('-o', '--extrapFile2015', help='Path to root file containing limit calculation results from 2015')
parser.add_argument('-H', '--mH', help='Mass of heavy boson (GeV)')
parser.add_argument('-S', '--mS', help='Mass of scalar (GeV)')
parser.add_argument('-p', '--plotName', help='File name for output limit plot')
args = parser.parse_args()
parser.print_help()

#Method to get TGraphAsymmErrors from TH1's
def getTGraphAsymErrs(nominalHist,upHist,dnHist):
  result = r.TGraphAsymmErrors()
  for ibin in range(0,nominalHist.GetNbinsX()+1):
     nominalY=nominalHist.GetBinContent(ibin)
     nominalX=nominalHist.GetBinCenter(ibin)
     XErr=nominalHist.GetBinWidth(ibin)/2.
     YErrUp=upHist.GetBinContent(ibin)
     YErrDn=dnHist.GetBinContent(ibin)
     result.SetPoint(ibin,nominalX,nominalY)
     result.SetPointEYhigh(ibin,abs(nominalY-YErrUp))
     result.SetPointEYlow(ibin,abs(nominalY-YErrDn))
     result.SetPointEXlow(ibin,XErr)
     result.SetPointEXhigh(ibin,XErr)
  return result

def getTGraph(hist):
  result = r.TGraph()
  for ibin in range(0,hist.GetNbinsX()+1):
    histY=hist.GetBinContent(ibin)
    histX=hist.GetBinCenter(ibin)
    result.SetPoint(ibin,histX,histY)
  return result

#Open the file
inFile = r.TFile(args.extrapFile2017)
inFile2015 = r.TFile(args.extrapFile2015)
#Set up the canvas
canvas = r.TCanvas('canvas', 'canvas', 2400, 1600)

# Get histograms
centralExp = inFile.Get('xsec_BR_95CL')
plus1      = inFile.Get('xsec_BR_events__p1')
plus2      = inFile.Get('xsec_BR_events__p2')
minus1     = inFile.Get('xsec_BR_events__n1')
minus2     = inFile.Get('xsec_BR_events__n2')
centralObs = inFile.Get('xsec_BR_events__limit')

l15centralExp = inFile2015.Get('xsec_BR_95CL')
l15plus1      = inFile2015.Get('xsec_BR_events__p1')
l15plus2      = inFile2015.Get('xsec_BR_events__p2')
l15minus1     = inFile2015.Get('xsec_BR_events__n1')
l15minus2     = inFile2015.Get('xsec_BR_events__n2')
l15centralObs = inFile2015.Get('xsec_BR_events__limit')

y_min2 = minus2.GetMinimum()
y_min1 = minus1.GetMinimum()
y_min = y_min2-(y_min1-y_min2)

tg_1s  = getTGraphAsymErrs(centralExp,plus1,minus1)
tg_2s  = getTGraphAsymErrs(centralExp,plus2,minus2)
tg_Exp = getTGraph(centralExp)
#tg_Obs = getTGraph(centralObs)

l15tg_1s  = getTGraphAsymErrs(l15centralExp,l15plus1,l15minus1)
l15tg_2s  = getTGraphAsymErrs(l15centralExp,l15plus2,l15minus2)
l15tg_Exp = getTGraph(l15centralExp)
l15tg_Obs = getTGraph(l15centralObs)

tg_2s.Draw("aC3")
tg_1s.Draw("C3 same ")
#tg_Obs.Draw("l same ")
tg_Exp.Draw("l same ")

tg_Exp.SetLineStyle(2)
tg_Exp.SetLineWidth(2)
#tg_Obs.SetLineStyle(1)
#tg_Obs.SetLineWidth(2)

tg_2s.SetMaximum(5e3)

tg_2s.SetFillColor(r.kYellow)
tg_1s.SetFillColor(r.kGreen-4)

l15tg_2s.Draw("C3 same")
l15tg_1s.Draw("C3 same ")
l15tg_Obs.Draw("l same ")
l15tg_Exp.Draw("l same ")

l15tg_Exp.SetLineStyle(2)
l15tg_Exp.SetLineWidth(1)
l15tg_Exp.SetLineColor(1)
l15tg_Obs.SetLineStyle(1)
l15tg_Obs.SetLineWidth(1)
l15tg_Obs.SetLineColor(1)

l15tg_2s.SetMaximum(5e3)

l15tg_2s.SetFillColor(r.kAzure-9)
l15tg_1s.SetFillColor(r.kPink+1)

x_min = 1.0
x_max = tg_2s.GetXaxis().GetXmax()

mH = args.mH
mS = args.mS

if mH == "1000":
  if mS == "400": x_min = 0.1
  if mS == "150": x_min = 0.04
  if mS == "50" : x_min = 0.04
if mH == "600":
  if mS == "150": x_min = 0.08
  if mS == "50" : x_min = 0.08
if mH == "400":
  if mS == "100": x_min = 0.1
  if mS == "50" : x_min = 0.04

tg_2s.GetXaxis().SetLimits(x_min, x_max)
tg_2s.SetMinimum(y_min)

tg_2s.GetXaxis().SetTitle('s proper decay length [m]')
tg_2s.GetYaxis().SetTitle('95% CL Upper Limit on #sigma #times BR [pb]')

tg_2s.Draw("aC4")
tg_1s.Draw("C4 same ")
#tg_Obs.Draw("l same ")
tg_Exp.Draw("l same ")

l15tg_2s.Draw("C4 same")
l15tg_1s.Draw("C4 same ")
l15tg_Obs.Draw("l same ")
l15tg_Exp.Draw("l same ")


r.gStyle.SetTextSize(0.05)
r.ATLASLabel(0.35,0.89,"Work in Progress",1)
r.gStyle.SetTextSize(0.035)
r.myText(0.35,0.85,1,"m_{H} = "+args.mH+" GeV, m_{s} = "+args.mS+" GeV")
r.myText(0.65,0.81,1,"#bf{2016}  33.0 fb^{-1}  #it{#sqrt{s}} = 13 TeV")
r.myBoxText(0.65,0.77,0.04,r.kYellow,"expected #pm 2#sigma")
r.myBoxText(0.65,0.73,0.04,r.kGreen-4,"expected #pm 1#sigma")
r.myBoxTextDash(0.65,0.69,0.04,r.kWhite,r.kBlack,"expected limit",2)
#r.myBoxTextDash(0.65,0.65,0.04,r.kWhite,r.kBlack,"observed limit",1)
r.myText(0.35,0.81,1,"#bf{2015}  3.2 fb^{-1}  #it{#sqrt{s}} = 13 TeV")
r.myBoxText(0.35,0.77,0.04,r.kAzure-9,"expected #pm 2#sigma")
r.myBoxText(0.35,0.73,0.04,r.kPink+1,"expected #pm 1#sigma")
r.myBoxTextDash(0.35,0.69,0.04,r.kWhite,r.kBlack,"expected limit",2)
r.myBoxTextDash(0.35,0.65,0.04,r.kWhite,r.kBlack,"observed limit",1)

canvas.SaveAs(args.plotName)
