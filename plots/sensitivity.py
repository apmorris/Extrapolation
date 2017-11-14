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
  result.SetPoint(ibin,errval,nominalY)
  result.SetPointEYhigh(ibin,abs(nominalY-YErrUp))
  result.SetPointEYlow(ibin,abs(nominalY-YErrDn))
  return result

def getTGraph(hist):
  result = r.TGraph()
  for ibin in range(0,hist.GetNbinsX()+1):
    histY=hist.GetBinContent(ibin)
    histX=hist.GetBinCenter(ibin)
    result.SetPoint(ibin,histX,histY)
  return result

#Open the files
nominalFile = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_nominal.root')
upFile = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_double' + args.variation + '.root')
downFile = r.TFile(args.directory + 'limit_mH' + args.mH + '_mS' + args.mS + '_lt5m_dv18_halve' + args.variation + '.root')

#Set up the canvas
canvas = r.TCanvas('canvas', 'canvas', 1200, 800)
#canvas = r.TCanvas('canvas', 'canvas', 2400, 1600)

# Get histograms
centralExp = nominalFile.Get('xsec_BR_95CL')
plus1      = nominalFile.Get('xsec_BR_events__p1')
minus1     = nominalFile.Get('xsec_BR_events__n1')

up_centralExp = upFile.Get('xsec_BR_95CL')
up_plus1      = upFile.Get('xsec_BR_events__p1')
up_minus1     = upFile.Get('xsec_BR_events__n1')

down_centralExp = downFile.Get('xsec_BR_95CL')
down_plus1      = downFile.Get('xsec_BR_events__p1')
down_minus1     = downFile.Get('xsec_BR_events__n1')

g_nom = getTGraphPoint(centralExp, plus1, minus1, args.ctau, args.error)
g_up = getTGraphPoint(up_centralExp, up_plus1, up_minus1, args.ctau, args.error*2)
g_down = getTGraphPoint(down_centralExp, down_plus1, down_minus1, args.ctau, args.error*0.5)

g_up.Draw("a P")
g_nom.Draw("P same")
g_down.Draw("P same")

g_up.SetLineColor(r.kGreen+2)
g_up.SetMarkerColor(r.kGreen+2)

g_down.SetLineColor(r.kRed+2)
g_down.SetMarkerColor(r.kRed+2)

#g_up.SetMinimum(0.99)
g_up.SetMaximum(0.1)

if args.variation == "MC": g_up.GetXaxis().SetTitle('Combined MC systematic')
if args.variation == "ABCD": g_up.GetXaxis().SetTitle('ABCD systematic')

g_up.GetYaxis().SetTitle('95% CL Upper Limit on #sigma #times BR [pb]')

g_up.Draw("a P")
g_nom.Draw("P same")
g_down.Draw("P same")

r.gStyle.SetTextSize(0.05)
r.ATLASLabel(0.2,0.87,"Internal",1)
r.gStyle.SetTextSize(0.025)
r.myText(0.2,0.83,1,"Sensitivity to "+args.variation+" systematic for")
r.myText(0.2,0.8,1,"m_{H} = "+args.mH+" GeV, m_{s} = "+args.mS+" GeV, c#tau = "+str(args.ctau)+" m")
r.myMarkerText(0.23,0.77,r.kGreen+2,20,"Doubled systematic",1)
r.myMarkerText(0.23,0.74,r.kBlack,20,"Nominal systematic",1)
r.myMarkerText(0.23,0.71,r.kRed+2,20,"Halved systematic",1)

canvas.SaveAs(args.plotName)
