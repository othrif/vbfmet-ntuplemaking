# Sherpa Truth/Parton Filtering

The ```VBFInvSherpaTruth ``` algorithm was written to do studies of SHERPA
parton-level (matrix element) filtering. It is designed to run over either
a TRUTH1 sample or a full AOD sample; the only real requirements are that
the sample being processed has both a truth jet collection and the *full*
truth particles collection.

Specifically, the algorithm attempts to calculate both truth and parton
level dijet mass ("MJJ"), in order to study the efficiency of matrix-element
level MJJ filtering. (In Sherpa, this filtering is implemented using a
[plugin](https://gitlab.cern.ch/brosser/MjjSherpaFilters)).

The algorithm produces a flat ROOT TTree (a "MiniNtuple"), with various
branches. See the "Output" section for more details.

The algorithm may be generalizable to other generators, but it is SHERPA
specific in a least a few places. See the "Details" section for what is done.

## Details

The algorithm does the following things:

* Selects truth jets to compute dijet mass.
* Iterate through the TruthParticles collection to select all "matrix element"
truth particles. In SHERPA, this is tricky (see below for details).
* Cluster the matrix element particles into "parton jets", then selects
two+ parton jets to compute "MJJ". (See below for details). Clustering is
done using anti-kT with delta R = 0.4 by default.
* Attempts to match truth and parton jets (by computing Delta R between them).

### Status 3 and 20 Particles

The HepMC standard status code for "matrix element" particles is "3". Therefore
to look at all matrix element particles, in theory we could just look for all
status 3 particles in the event record.

Unfortunately, when generating NLO events with Sherpa, it's not quite that
simple. Sherpa implements the MC@NLO technique, in which NLO events can have
two different initial conditions:

* H events, where the first emission of the event was generated as part of
selecting the phase space point.

* S events, where there are no initial emissions included in the matrix
element.

However, because of the way the initial emission is handled in a S event,
the initial emission *will* be included in the status 3 particles. So,
Sherpa defines another status code-- status 20-- for "S event before initial
emission". H events don't have these status 20 particles; therefore the
algorithm to select truth particles uses status 20 particles if they are
there; and if they are not, uses status 3 particles when computing
quantities in the "parton" branches.

Just to be safe, however, "status3" and "status20" branches are written
for *every* event, so the "status3" partons are available for inspection
even for H events.

(The "clusterPartonCode" variable is set to "3" or "20" depending on
whether or not it was a S or H event).

### Selecting Jets

The simplest way to compute MJJ is to take the two jets in the event with the
largest pT. This is what is done in the plugin implementing MJJ filtering.

However, this may not be the "best" way to select jets at the parton-level.
To account for this, a variety of different schemes are tried by this algorithm:

* "Lead": take the two jets with highest pT.
* "Max": take the two jets which give the highest MJJ, regardless of pT.
* "Best": take the two jets which are closest to the truth MJJ.
* "MaxAll": like "Max", but sums over all possible jets.

All four techniques are implemented for both truth and parton jets.

## Output

The output ntuples contain five different sets of variables:

* "truth": computed from truth jets.
* "parton": computed from partons and parton jets.
* "status3": computed from status 3 partons.
* "status20": computed from status 20 partons.
* "postShower": computed from status 11 partons produced by parton shower
(this is a new experimental addition).

For everything but the truth jets, the full collection of all particle kinematics
and PDG IDs are written to the output ntuple.

Then, for each of the above sets of variables, jet kinematics are written.

Each of the MJJ computation schemes described in "Selecting Jets" are
used to compute dijet mass for all these sets of variables. In
addition to just "Mjj", "deta" and "dphi" are computed as well.

TODO: write more.

## Running

You can select this algorithm by invoking the runner script as follows:

```
runVBF.py -a VBFInvSherpaTruth
```

In addition to the standard options for running the VBF analysis framework,
the algorithm takes the following extra arguments:

```
  --parton-pt PARTONPT  Parton jet pT cut when selecting jets for mjj, in GeV.
  --truth-pt TRUTHPT    Truth jet pT cut when selecting jets for mjj, in GeV.
  --no-cluster-partons  Skip clustering the partons when selecting parton
                        jets.
  --antikt-dr ANTIKTDR  Value of delta R to use when clustering partons with
                        anti-KT.
```
