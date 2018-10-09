#!/usr/bin/env python

# sherpa_print_event_record.py - print the truth event record for a given event
# as stored in a ntuple/tree produed from the "VBFInvSherpaTruth" algorithm.

# The event record is printed out in markdown format for easy conversion to
# HTML or PDF.

# Ben Rosser <bjr@sas.upenn.edu>

import argparse
import math
import os
import sys
import random

import ROOT

def printPartons(tree, numPartons, varBase, withMass=False):

	partons = {}
	beams = []
	neutrinos = []

	for i in range(numPartons):
		pdgId = eval("tree." + varBase + "PID[i]")
		pt = eval("tree." + varBase + "Pt[i]") / 1000.
		eta = eval("tree." + varBase + "Eta[i]")
		phi = eval("tree." + varBase + "Phi[i]")
		e = eval("tree." + varBase + "E[i]") / 1000.

		# Also ignore the neutrinos I guess.
		if pt != 0 and not ROOT.TMath.Abs(pdgId) in [12, 14, 16]:
			parton = ROOT.TLorentzVector()
			parton.SetPtEtaPhiE(pt, eta, phi, e)
			partons[i+1] = parton

		if pt == 0:
			beam = ROOT.TLorentzVector()
			beam.SetPxPyPzE((-1)**i * e, 0, 0, e)
			beams.append(beam)

		if i == 2 or i == 3:
			neutrino = ROOT.TLorentzVector()
			neutrino.SetPtEtaPhiE(pt, eta, phi, e)
			neutrinos.append(neutrino)

		if i == 4 or i == 2:
			print("")

		if withMass:
			m = eval("tree." + varBase + "Mass[i]")
			print("\tParticle %d: PDG ID %d\t(Pt, Eta, Phi, E) = (%f, %f, %f, %f); Mass = %f" % (i+1, pdgId, pt, eta, phi, e, m))
		else:
			print("\tParticle %d: PDG ID %d\t(Pt, Eta, Phi, E) = (%f, %f, %f, %f)" % (i+1, pdgId, pt, eta, phi, e))

	return partons

def printTruthJets(tree, numTruthJets, pt_cut=-1):

	jets = {}

	for i in range(numTruthJets):
		pt = tree.truth_jetPt[i] / 1000.
		eta = tree.truth_jetEta[i]
		phi = tree.truth_jetPhi[i]
		e = tree.truth_jetE[i] / 1000.

		jet = ROOT.TLorentzVector()
		jet.SetPtEtaPhiE(pt, eta, phi, e)

		if pt >= pt_cut:
			print("\tTruth Jet %d:\t(Pt, Eta, Phi, E) = (%f, %f, %f, %f)" % (i+1, pt, eta, phi, e))
			jets[i+1] = jet

	return jets

def printPartonJets(tree, numTruthJets, varBase, pt_cut=-1):

	jets = {}
	for i in range(numTruthJets):
		pt = eval("tree." + varBase + "Pt[i]") / 1000.
		eta = eval("tree." + varBase + "Eta[i]")
		phi = eval("tree." + varBase + "Phi[i]")
		e = eval("tree." + varBase + "E[i]") / 1000.

		jet = ROOT.TLorentzVector()
		jet.SetPtEtaPhiE(pt, eta, phi, e)

		if pt >= pt_cut:
			print("\tParton Jet %d:\t(Pt, Eta, Phi, E) = (%f, %f, %f, %f)" % (i+1, pt, eta, phi, e))
			jets[i+1] = jet

	return jets

def printMatchedJetsPartons(jets, status20s, status3s):

	# This is awkwardly hard-coded.
	matched3s = []
	matched20s = []
	for jetIndex, jet in jets.items():
		print("\tTruth Jet %d:" % jetIndex)

		hasMatched = False
		for partonIndex, parton in status20s.items():
			deltaR = jet.DeltaR(parton)

			if partonIndex in matched20s:
				continue

			if deltaR <= 0.4:
				print("\t\tMatches Status 20 Particle %d with Delta R = %f" % (partonIndex, deltaR))
				matched20s.append(partonIndex)
				hasMatched = True

		if not hasMatched:
			for partonIndex, parton in status20s.items():
				if partonIndex in matched20s:
					continue

				deltaR = jet.DeltaR(parton)
				print("\t\tDOES NOT match Status 20 Particle %d with Delta R = %f" % (partonIndex, deltaR))

		hasMatched = False
		for partonIndex, parton in status3s.items():
			deltaR = jet.DeltaR(parton)

			if partonIndex in matched3s:
				continue

			if deltaR <= 0.4:
				print("\t\tMatches Status 3 Particle %d with Delta R = %f" % (partonIndex, deltaR))
				matched3s.append(partonIndex)
				hasMatched = True

		if not hasMatched:
			for partonIndex, parton in status3s.items():
				if partonIndex in matched3s:
					continue

				deltaR = jet.DeltaR(parton)
				print("\t\tDOES NOT match Status 3 Particle %d with Delta R = %f" % (partonIndex, deltaR))

def printMatchedJets(jets, otherJets):

	# This is awkwardly hard-coded.
	matched = []
	for jetIndex, jet in jets.items():
		print("\tTruth Jet %d:" % jetIndex)

		hasMatched = False
		for partonIndex, parton in otherJets.items():
			deltaR = jet.DeltaR(parton)

			if partonIndex in matched:
				continue

			if deltaR <= 0.4:
				print("\t\tMatches Post-Shower Jet %d with Delta R = %f" % (partonIndex, deltaR))
				matched.append(partonIndex)
				hasMatched = True

		if not hasMatched:
			for partonIndex, parton in otherJets.items():
				if partonIndex in matched:
					continue

				deltaR = jet.DeltaR(parton)
				print("\t\tDOES NOT match Post-Shower Jet %d with Delta R = %f" % (partonIndex, deltaR))

def printSingleEvent(tree, event, withMass=False, truthPt=10):
	tree.GetEntry(event)

	print("# Event %d (Event Number %d):" % (event, tree.EventNumber))

	# Is this status 3 or 20 (S or H)?
	if tree.clusterPartonCode == 3:
		print("\tH Event (status 3 partons)\n")
	else:
		print("\tS Event (status 20 partons)\n")

	print("\tParton Mjj = %f GeV, Truth Mjj = %f GeV" % (tree.parton_Mjj_lead, tree.truth_Mjj_lead))
	print("\tParton HT = %f GeV, Parton PTV = %f GeV" % (tree.parton_HT, tree.parton_PTV))
	print("\tEvent Weight = %f" % tree.EventWeight)

	print("")

	# Print the status 3 and status 20 truth particles.
	if tree.clusterPartonCode == 20:
		print("## Status 20 Particles")
		status20s = printPartons(tree, tree.status20_numParticles, "status20_", withMass=withMass)
		print("")
	print("## Status 3 Particles")
	status3s = printPartons(tree, tree.status3_numParticles, "status3_", withMass=withMass)
	print("")

	# Print the status 3 and status 20 _jets_ only.
	if tree.clusterPartonCode == 20:
		print("## Status 20 Jets")
		printPartonJets(tree, tree.status20_numJets, "status20_jet")
		print("")
	print("## Status 3 Jets")
	printPartonJets(tree, tree.status3_numJets, "status3_jet")
	print("")

	# Print the post-shower status 11 jets.
	print("## Post-Shower Status 11 Jets (Down to %.1f GeV)" % truthPt)
	status11_jets = printPartonJets(tree, tree.postShower_numJets, "postShower_jet", pt_cut=truthPt)
	print("")

	# Print the truth jets.
	print("## Truth Jets (Down to %.1f GeV)" % truthPt)
	jets = printTruthJets(tree, tree.truth_numJets, pt_cut=truthPt)
	print("")

	# Print matchings.
	if tree.clusterPartonCode != 20:
		status20s = {}
	print("## Truth-Parton Matching")
	printMatchedJetsPartons(jets, status20s, status3s)
	print("")

	# Do post-shower truth matching.
	print("## Truth + Post-Shower Matching")
	printMatchedJets(jets, status11_jets)
	print("")

	# Print other possible mjj. Re-add support for this later.
#	print("## Other Parton-Level Mjj")
#	print("\tBiggest Parton Mjj (Jets %d, %d) = %f GeV" % (tree.parton_maxLead+1, tree.parton_maxSublead+1, tree.partonMjj_max))
#	print("\tClosest Parton Mjj to Truth (Jets %d, %d) = %f GeV" % (tree.parton_bestLead+1, tree.parton_bestSublead+1, tree.partonMjj_best))

#	if tree.clusterPartonCode == 20:
#		print("\tStatus 3: Biggest Parton Mjj (Jets %d, %d) = %f GeV" % (tree.parton3_maxLead+1, tree.parton3_maxSublead+1, tree.parton3Mjj_max))
#		print("\tStatus 3: Closest Parton Mjj to Truth (Jets %d, %d) = %f GeV" % (tree.parton3_bestLead+1, tree.parton3_bestSublead+1, tree.parton3Mjj_best))

def main():
	parser = argparse.ArgumentParser(description='Print truth event record from Sherpa sample.')
	parser.add_argument('filename', metavar='FILENAME', type=str, help='ROOT file to run over.')

	# For now, just support a single event.
	parser.add_argument('-e', '--event', default=0, dest='event', type=int, help='Event index to print out.')
	parser.add_argument('-p', '--pt', default=10.0, dest='ptcut', type=float, help='pT below which to ignore truth/post-shower jets.')

	parser.add_argument('-r', '--random', action='store_true', dest='random', help='Print out ten random events from file.')

	parser.add_argument('-m', '--mass', action='store_true', dest='mass', help='Print partons out with mass.')

	args = parser.parse_args()

	# Make sure the file exists.
	path = os.path.abspath(args.filename)
	if not os.path.exists(path):
		print("Error: input file " + str(path) + " does not exist.")
		sys.exit(1)

	# Print single event record.
	tfile = ROOT.TFile(path)
	tree = tfile.Get("MiniNtuple")

	if not args.random:

		if args.event >= tree.GetEntries():
			print("Error: event index %d outside bounds of tree (0, %d)." % (args.event, tree.GetEntries()))
			sys.exit(1)

		printSingleEvent(tree, args.event, withMass=args.mass, truthPt=args.ptcut)

	else:
		entries = tree.GetEntries()
		# Make this random!
		for i in range(10):
			entry = random.randint(0, entries)
			printSingleEvent(tree, entry, withMass=args.mass, truthPt=args.ptcut)

if __name__ == '__main__':
	main()

parser = argparse.ArgumentParser(description='Process some integers.')
