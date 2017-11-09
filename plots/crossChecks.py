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
color = [r.kBlack,r.kOrange+7,r.kCyan,r.kMagenta,r.kSpring-1,r.kBlue-9,r.kRed,r.kBlue,r.kViolet-7,r.kGreen+3,r.kRed+3]

#Set up the canvas
#canvas = r.TCanvas('canvas', 'canvas', 2400, 1600)
canvas = r.TCanvas('canvas', 'canvas', 1200, 800)
hs.Draw("nostack")

r.gStyle.SetTextSize(0.05)
r.ATLASLabel(0.19,0.88,"Internal",1)
r.gStyle.SetTextSize(0.04)

i=0
for mass in massPts:
  mH = mass.split('mH')[1].split('_')[0]
  mS = mass.split('mS')[1]
  print "Cross-checks for mH", mH, "and mS", mS, "being carried out"
  if int(mH) < 399 : filename = r.TFile(args.extrapPath + "/extrap_" + mass + "_lt5m_dv18_sel1.root")
  if int(mH) > 399 : filename = r.TFile(args.extrapPath + "/extrap_" + mass + "_lt5m_dv18_sel2.root")
  print "From file", filename
  histo = filename.Get('h_res_eff_A')
  print "Maximum is", histo.GetMaximum()
  r.gROOT.cd()
  hnew = histo.Clone()
  hnew.SetLineColor(color[i])
  hnew.SetMarkerColor(color[i])
  hs.Add(hnew)
  if (int(mH)==400 and int(mS)==50): xmin,cx,cy,ex,ey = 0.01,array('f',[0.7]),array('f',[0.017039]),array('f',[1.26]),array('f',[0.0121415])
  if (int(mH)==400 and int(mS)==100): xmin,cx,cy,ex,ey = 0.01,array('f',[1.46]),array('f',[0.016586]),array('f',[2.64]),array('f',[0.0126171])
  if (int(mH)==600 and int(mS)==50): xmin,cx,cy,ex,ey = 0.01,array('f',[0.52]),array('f',[0.035142]),array('f',[0.96]),array('f',[0.0225140])
  if (int(mH)==600 and int(mS)==150): xmin,cx,cy,ex,ey = 0.01,array('f',[1.72]),array('f',[0.033143]),array('f',[3.14]),array('f',[0.0231199])
  if (int(mH)==1000 and int(mS)==50): xmin,cx,cy,ex,ey = 0.01,array('f',[0.38]),array('f',[0.052205]),array('f',[0.67]),array('f',[0.0330391])
  if (int(mH)==1000 and int(mS)==150): xmin,cx,cy,ex,ey = 0.01,array('f',[1.17]),array('f',[0.049109]),array('f',[2.11]),array('f',[0.0317666])
  if (int(mH)==1000 and int(mS)==400): xmin,cx,cy,ex,ey = 0.01,array('f',[3.96]),array('f',[0.023929]),array('f',[7.20]),array('f',[0.0179058])
  print cx,cy,ex,ey
  closure = r.TGraph(1,cx,cy)
  closure.SetLineColor(r.kMagenta-3)
  closure.SetMarkerColor(r.kMagenta-3)
  closure.SetMarkerStyle(33)
  closure.SetMarkerSize(2.5)
  closure.Draw("p same")
  extrapolation = r.TGraph(1,ex,ey)
  extrapolation.SetLineColor(r.kAzure-3)
  extrapolation.SetMarkerColor(r.kAzure-3)
  extrapolation.SetMarkerStyle(23)
  extrapolation.SetMarkerSize(2)
  extrapolation.Draw("p same")
  r.myText(0.19,0.83,1,"#it{m_{H}} = "+mH+" GeV, #it{m_{s}} = "+mS+" GeV")
  r.myBoxTextDash(0.19,0.79,0.03,r.kWhite,color[i]," Extrapolation",1)
  r.myMarkerText(0.23,0.75,r.kMagenta-3,33,"LF = 5m",2.5);
  r.myMarkerText(0.23,0.71,r.kAzure-3,23,"LF = 9m",2);
  i+=1

#hs.Draw("nostack")
hs.Modified()
hs.GetXaxis().SetTitle("#it{s} proper decay length [m]")
hs.GetYaxis().SetTitle("Global Efficiency")
maximum = hs.GetMaximum("nostack")
hs.SetMaximum(maximum*1.2)
hs.Modified()
canvas.SaveAs(args.outputName)


