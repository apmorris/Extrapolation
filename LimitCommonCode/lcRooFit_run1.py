#!/usr/bin/env python
import ROOT as r
import math

mH300_mS50  = {'paper':0.16,'exp':444.,'expstaterr':11./444.,'acceptance':0.609,'expsysterr':0.136}
mH600_mS150 = {'paper':0.26,'exp':41. ,'expstaterr':2./41. ,'acceptance':0.386,'expsysterr':0.163}
mH600_mS50  = {'paper':0.31,'exp':35. ,'expstaterr':1./35. ,'acceptance':0.33,'expsysterr':0.166}
mH900_mS150 = {'paper':0.24,'exp':4.6 ,'expstaterr':0.2/4.6,'acceptance':0.397,'expsysterr':0.172}
mH900_mS50  = {'paper':0.33,'exp':3.5 ,'expstaterr':0.1/3.5,'acceptance':0.304,'expsysterr':0.172}

s=mH600_mS150


tf = r.TFile.Open("limit_result.root")
ws = tf.Get('wspace')
ws.var('NA').setVal(24)
ws.var('NA').setConstant()
ws.var('NB').setConstant()
ws.var('NC').setConstant()
ws.var('ND').setConstant()
model=ws.pdf('modelAOnly')
#model=ws.pdf('model')
obsA=ws.pdf('obsA')
mu = ws.var('mu')
mu.setVal(1e-3)
constraint_lumi=ws.obj('constraint_lumi')
constraint_S=ws.obj('constraint_S')
constraint_Q=ws.obj('constraint_Q')
nll=r.RooFormulaVar("nll","-2*TMath::Log(@0)",r.RooArgList(model))
lh=r.RooFormulaVar("lh","@0",r.RooArgList(model))
model.Print("")
ws.var('Nq').setVal(23.5)
ws.var('Nq').setConstant()
ws.var('Ns0').setVal(s['exp'])
#ws.var('effB').setVal(0.212)
#ws.var('effC').setVal(0.218)
#ws.var('effD').setVal(0.075)
nom_sigma_lumi = ws.var('nom_sigma_lumi')
nom_sigma_lumi.setVal(1.0*nom_sigma_lumi.getVal())
nom_sigma_S = ws.var('nom_sigma_S')
nom_sigma_S.setVal((s['expstaterr']**2 + s['expsysterr']**2)**0.5)
nom_sigma_Q = ws.var('nom_sigma_Q')
nom_sigma_Q.setVal(0.34)
print "Floatign Params:"
nll.getParameters(r.RooArgSet()).selectByAttrib("Constant",r.kFALSE).Print("V")
print "Fixed Params:"
nll.getParameters(r.RooArgSet()).selectByAttrib("Constant",r.kTRUE).Print("V")
m=r.RooMinuit(nll)
fltParams = nll.getParameters(r.RooArgSet()).selectByAttrib("Constant",r.kFALSE)
allParams = nll.getParameters(r.RooArgSet())
fitRes= m.fit("MIGRAD")
muHatNLL=fitRes.minNll()
muHat=mu.getVal()
postFit= r. RooArgSet()
allParams.snapshot(postFit)
postFit.Print('V')
print " muHatNLL ", muHatNLL, " muHat ", muHat

minMu=1e-6
maxMu=10.0
npMu=2000
muVals=[]
tmpMu=minMu
while (tmpMu < maxMu):
  muVals.append(tmpMu)
  tmpMu+=abs(minMu-maxMu)/npMu
f = open('values.log', 'w')
tg = r.TGraph()
mu.setConstant() 
counter=0
m=r.RooMinuit(nll)
muErrs=[-999,-999] #1 sigma, 2 sigma
for muVal in muVals:
  mu.setVal(muVal)
  thisNLL= m.fit("MIGRAD").minNll()
  tg.SetPoint(counter,muVal,thisNLL-  muHatNLL)
  line = fltParams.Print()
  #for fltP in fltParams:
  #  line+=" %s=%f " % (fltP.GetName(), fltP.getVal())
  f.write("mu=%f (muHat=%f) - thisNLL=%f, bestNLL=%f, lh=%f, obsA=%f, constraint_lumi=%f, constraint_Q=%f. constraint_S=%f \n" % (muVal, muHat,  thisNLL, muHatNLL,lh.getVal(), obsA.getVal(), constraint_lumi.getVal(), constraint_Q.getVal(), constraint_S.getVal()))
  for obs in ['NexpA','NbQA', 'Nq', 'alpha_Q','NsA','Ns0','alpha_lumi','alpha_S','lumi']:
    f.write("-->%s=%f\n" % (obs, ws.obj(obs).getVal()))
  
  
  counter+=1
  if abs(thisNLL-  muHatNLL)<1 : muErrs[0]=muVal 
  if abs(thisNLL-  muHatNLL)<4 : muErrs[1]=muVal 
  if abs(thisNLL-  muHatNLL)>5 : break 


print "MU = %f +/- %f (1sigma) +/- %f (2sigma) " % (muHat, muErrs[0]-muHat, muErrs[1]-muHat )
print " Naive 95%% CL r=%f, XS*BR=%f (compare to paper %f)" %(muErrs[1] , muErrs[1]* s['exp']/(0.01*s['acceptance']) /20300., s['paper'])
c= r.TCanvas("c","c",500,500)
tg.Draw('apl')
c.SaveAs('test.pdf')
#mu.setVal(muHat)
#thisNLL= m.fit("MIGRAD").minNll()
#nll.getParameters(r.RooArgSet()).selectByAttrib("Constant",r.kFALSE).Print("V")
#postFit.Print("V")
