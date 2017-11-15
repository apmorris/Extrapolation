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
#r.gStyle.SetOptLogy(1)
#r.gStyle.SetOptLogx(1)
r.gStyle.SetLineWidth(2)
r.gStyle.SetEndErrorSize(4)

#Parse command line
parser = argparse.ArgumentParser(description='Make plots for variations in the systematics')
parser.add_argument('-d', '--directory', help='Path to directory containing limit calculation results for sensitivity tests')
parser.add_argument('-t', '--variation', help='Type of variation considered (ABCD/MC)')
parser.add_argument('-H', '--mH', help='Mass of heavy boson (GeV)')
parser.add_argument('-S', '--mS', help='Mass of scalar (GeV)')
parser.add_argument('-c', '--ctau', help='Generated ctau for sample', type=float)
parser.add_argument('-e', '--error', help='Nominal error value', type=float)
parser.add_argument('-x', '--dimensions', help='Ranges for axes (xmin,xmax,ymin,ymax)')
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

def getTGraphPoint(nominalHist,upHist,dnHist,xval,errval):
  result = r.TGraphAsymmErrors()
  ibin = nominalHist.FindBin(xval)
  nominalY=nominalHist.GetBinContent(ibin)
  YErrUp=upHist.GetBinContent(ibin)
  YErrDn=dnHist.GetBinContent(ibin)
  result.SetPoint(0,errval,nominalY)
  result.SetPointEYhigh(0,abs(nominalY-YErrUp))
  result.SetPointEYlow(0,abs(nominalY-YErrDn))
  return result

def getTGraph(hist):
  result = r.TGraph()
  for ibin in range(0,hist.GetNbinsX()+1):
    histY=hist.GetBinContent(ibin)
    histX=hist.GetBinCenter(ibin)
    result.SetPoint(ibin,histX,histY)
  return result

#Open the files
file200     = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_200' + args.variation + '.root')
file180     = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_180' + args.variation + '.root')
file160     = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_160' + args.variation + '.root')
file140     = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_140' + args.variation + '.root')
file120     = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_120' + args.variation + '.root')
nominalFile = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_nominal.root')
file80      = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_80' + args.variation + '.root')
file60      = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_60' + args.variation + '.root')
file40      = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_40' + args.variation + '.root')
file20      = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_20' + args.variation + '.root')
file0       = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_0' + args.variation + '.root')

#Set up the canvas
canvas = r.TCanvas('canvas', 'canvas', 1200, 800)
#canvas = r.TCanvas('canvas', 'canvas', 2400, 1600)

files = [file0, file20, file40, file60, file80, nominalFile, file120, file140, file160, file180, file200]
errorVals = [0*args.error, 0.2*args.error, 0.4*args.error, 0.6*args.error, 0.8*args.error, args.error, 1.2*args.error, 1.4*args.error, 1.6*args.error, 1.8*args.error, 2*args.error]
plus2Val = []
minus2Val = []
graph1 = r.TGraphAsymmErrors()
graph2 = r.TGraphAsymmErrors()

for i in range(0,len(errorVals)-1):
  print i
  histLimit = files[i].Get('xsec_BR_95CL')
  histPlus1 = files[i].Get('xsec_BR_events__p1')
  histMinus1 = files[i].Get('xsec_BR_events__n1')
  histPlus2 = files[i].Get('xsec_BR_events__p2')
  histMinus2 = files[i].Get('xsec_BR_events__n2')
  ibin = histLimit.FindBin(args.ctau)
  limitVal=histLimit.GetBinContent(ibin)
  plus1Val=histPlus1.GetBinContent(ibin)
  plus2Val.append(histPlus2.GetBinContent(ibin))
  minus1Val=histMinus1.GetBinContent(ibin)
  minus2Val.append(histMinus2.GetBinContent(ibin))
  graph1.SetPoint(i,errorVals[i],limitVal)
  graph1.SetPointEYhigh(i,abs(limitVal-plus1Val))
  graph1.SetPointEYlow(i,abs(limitVal-minus1Val))
  graph2.SetPoint(i,errorVals[i],limitVal)
  graph2.SetPointEYhigh(i,abs(limitVal-plus2Val[i]))
  graph2.SetPointEYlow(i,abs(limitVal-minus2Val[i]))

graph1.Draw("A""P")
graph2.Draw("[]")
graph2.SetMarkerSize(20)

xmin = 0
xmax = args.error*2.1
ydiffmax= max(plus2Val)-min(minus2Val)
print ydiffmax

graph1.SetMinimum(min(minus2Val) - ydiffmax/6.)
graph1.SetMaximum(max(plus2Val) + ydiffmax/2.)
graph1.GetXaxis().SetLimits(xmin,xmax)

if args.variation == "MC": graph1.GetXaxis().SetTitle('Combined MC systematic')
if args.variation == "ABCD": graph1.GetXaxis().SetTitle('ABCD systematic')

graph1.GetYaxis().SetTitle('95% CL Upper Limit on #sigma #times BR [pb]')

graph1.Draw("A""P")
graph2.Draw("[]")

r.gStyle.SetTextSize(0.05)
r.ATLASLabel(0.2,0.87,"Internal",1)
r.gStyle.SetTextSize(0.025)
r.myText(0.2,0.83,1,"Sensitivity to "+args.variation+" systematic, m_{H} = "+args.mH+" GeV, m_{s} = "+args.mS+" GeV")
r.myText(0.2,0.8,1,"Nominal error = "+str(args.error)+", c#tau = "+str(args.ctau)+" m")
#r.myMarkerText(0.23,0.77,r.kGreen+2,20,"Doubled systematic",1)
#r.myMarkerText(0.23,0.74,r.kBlack,20,"Nominal systematic",1)
#r.myMarkerText(0.23,0.71,r.kRed+2,20,"Halved systematic",1)

canvas.SaveAs(args.plotName)
