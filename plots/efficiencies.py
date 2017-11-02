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
r.gStyle.SetOptLogy(0)
r.gStyle.SetOptLogx(1)
r.gStyle.SetLineWidth(1)
r.gStyle.SetMarkerSize(0.05)
r.gStyle.SetMarkerStyle(20)

#Parse command line
parser = argparse.ArgumentParser(description='Makes a plot of efficiencies as a function of lifetime')
parser.add_argument('-p', '--extrapPath', help='Path to root file containing extrapolation results')
parser.add_argument('-m', '--massPairs', help='List of mass points to look at, e.g. mH400_mS50.mH600_mS150.mH1000_mS400')
parser.add_argument('-o', '--outputName', help='File name for output limit plot')
args = parser.parse_args()
parser.print_help()

massPts = str(args.massPairs).split('.')
hs = r.THStack("hs", "Lifetime [m]")
color = [r.kOrange+7,r.kCyan,r.kMagenta,r.kSpring-1,r.kAzure+6,r.kBlack,r.kRed,r.kBlack,r.kBlack,r.kBlack]

#Set up the canvas
#canvas = r.TCanvas('canvas', 'canvas', 2400, 1600)
canvas = r.TCanvas('canvas', 'canvas', 1200, 800)
hs.Draw("nostack")

r.gStyle.SetTextSize(0.05)
r.ATLASLabel(0.19,0.88,"Work in Progress",1)
r.gStyle.SetTextSize(0.025)

i=0
for mass in massPts:
  mH = mass.split('mH')[1].split('_')[0]
  mS = mass.split('mS')[1]
  print "Efficiency for mH", mH, "and mS", mS, "being calculated"
  filename = r.TFile(args.extrapPath + "/extrap_" + mass + "_lt5m_dv18.root")
  print "From file", filename
  histo = filename.Get('h_res_eff_A')
  print "Maximum is", histo.GetMaximum()
  r.gROOT.cd()
  hnew = histo.Clone()
  hnew.SetLineColor(color[i])
  hnew.SetMarkerColor(color[i])
  hs.Add(hnew)
  r.myBoxTextDash(0.19,0.84-(0.03*i),0.02,r.kWhite,color[i],"#it{m_{#Phi}} = " + mH + " GeV, #it{m_{S}} = " + mS + " GeV",1)
  i+=1

#hs.Draw("nostack")
hs.Modified()
hs.GetXaxis().SetTitle("Lifetime [m]")
hs.GetYaxis().SetTitle("Global Efficiency")
maximum = hs.GetMaximum("nostack")
hs.SetMaximum(maximum*1.2)
hs.Modified()
canvas.SaveAs(args.outputName)


