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
parser = argparse.ArgumentParser(description='Make limit plot with result of ExtrapLimitFinder code')
parser.add_argument('-e', '--extrapFile', help='Path to root file containing limit calculation results')
parser.add_argument('-H', '--mH', help='Mass of heavy boson (GeV)')
parser.add_argument('-S', '--mS', help='Mass of scalar (GeV)')
parser.add_argument('-p', '--plotName', help='File name for output limit plot')
parser.add_argument('-x', '--selection', help='Selection used in limit')
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
inFile = r.TFile(args.extrapFile)

#Set up the canvas
canvas = r.TCanvas('canvas', 'canvas', 2400, 1600)

# Get histograms
centralExp = inFile.Get('xsec_BR_95CL')
plus1      = inFile.Get('xsec_BR_events__p1')
plus2      = inFile.Get('xsec_BR_events__p2')
minus1     = inFile.Get('xsec_BR_events__n1')
minus2     = inFile.Get('xsec_BR_events__n2')
centralObs = inFile.Get('xsec_BR_events__limit')

y_min2 = minus2.GetMinimum()
y_min1 = minus1.GetMinimum()
y_min = y_min2-(y_min1-y_min2)

tg_1s  = getTGraphAsymErrs(centralExp,plus1,minus1)
tg_2s  = getTGraphAsymErrs(centralExp,plus2,minus2)
tg_Exp = getTGraph(centralExp)
#tg_Obs = getTGraph(centralObs)

tg_2s.Draw("aC3")
tg_1s.Draw("C3 same ")
#tg_Obs.Draw("l same ")
tg_Exp.Draw("l same ")

tg_Exp.SetLineStyle(2)
tg_Exp.SetLineWidth(2)
#tg_Obs.SetLineStyle(1)
#tg_Obs.SetLineWidth(2)

tg_2s.SetFillColor(r.kYellow)
tg_1s.SetFillColor(r.kGreen-4)

#tg_2s.SetFillColor(r.kAzure-9)
#tg_1s.SetFillColor(r.kPink+1)

x_min = 0.001
x_max = tg_2s.GetXaxis().GetXmax()
y_max = 5e3

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

tg_2s.GetXaxis().SetLimits(x_min+0.5*x_min, x_max)
tg_1s.GetXaxis().SetLimits(x_min+0.5*x_min, x_max)

tg_Exp.GetXaxis().SetLimits(x_min, x_max)
tg_Exp.SetMinimum(y_min)
tg_Exp.SetMaximum(y_max)

tg_Exp.GetXaxis().SetTitle('s proper decay length [m]')
tg_Exp.GetYaxis().SetTitle('95% CL Upper Limit on #sigma #times BR [pb]')

#frame = canvas.DrawFrame(c_min,y_min,x_max,y_max)

tg_Exp.Draw("al")
tg_2s.Draw("C3 same ")
tg_1s.Draw("C3 same ")
#tg_Obs.Draw("l same ")
tg_Exp.Draw("l same ")

r.gPad.RangeAxis(x_min, y_min, x_max, y_max)
r.gPad.RedrawAxis()
canvas.Modified()

r.gStyle.SetTextSize(0.05)
r.ATLASLabel(0.5,0.85,"Work in Progress",1)
r.gStyle.SetTextSize(0.035)
r.myText(0.5,0.8,1,"m_{H} = "+args.mH+" GeV, m_{s} = "+args.mS+" GeV, Selection "+args.selection)
r.myText(0.5,0.76,1,"33.0 fb^{-1}  #it{#sqrt{s}} = 13 TeV")
r.myBoxText(0.5,0.72,0.04,r.kYellow,"expected #pm 2#sigma")
r.myBoxText(0.5,0.68,0.04,r.kGreen-4,"expected #pm 1#sigma")
#r.myBoxText(0.5,0.72,0.04,r.kAzure-9,"expected #pm 2#sigma")
#r.myBoxText(0.5,0.68,0.04,r.kPink+1,"expected #pm 1#sigma")
r.myBoxTextDash(0.5,0.64,0.04,r.kWhite,r.kBlack,"expected limit",2)
#r.myBoxTextDash(0.5,0.6,0.04,r.kWhite,r.kBlack,"observed limit",1)

canvas.SaveAs(args.plotName)
