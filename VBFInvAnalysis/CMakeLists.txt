# Set the name of the package:
atlas_subdir( VBFInvAnalysis )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC
   Control/AthToolSupport/AsgTools
   PhysicsAnalysis/AnalysisCommon/PATCore
   PhysicsAnalysis/D3PDTools/EventLoop
   PhysicsAnalysis/D3PDTools/EventLoopGrid
   PhysicsAnalysis/D3PDTools/EventLoopAlgs
   PhysicsAnalysis/Interfaces/AsgAnalysisInterfaces
   Control/AthContainers
   Control/CxxUtils
   Control/xAODRootAccess
   Event/xAOD/xAODBase
   Event/xAOD/xAODCore
   Event/xAOD/xAODEgamma
   Event/xAOD/xAODEventInfo
   Event/xAOD/xAODTracking
   Event/xAOD/xAODJet
   Event/xAOD/xAODMuon
   Event/xAOD/xAODTau
   Event/xAOD/xAODCaloEvent
   Event/xAOD/xAODTruth
   Event/xAOD/xAODMissingET
   Event/xAOD/xAODBTaggingEfficiency
   Event/xAOD/xAODTrigger
   Event/xAOD/xAODCutFlow
   Event/xAOD/xAODMetaData
   PhysicsAnalysis/ElectronPhotonID/ElectronPhotonSelectorTools
   PhysicsAnalysis/TauID/TauAnalysisTools
   PhysicsAnalysis/ElectronPhotonID/ElectronEfficiencyCorrection
   PhysicsAnalysis/Interfaces/EgammaAnalysisInterfaces
   Trigger/TrigAnalysis/TrigDecisionTool
   Trigger/TrigConfiguration/TrigConfxAOD
   Trigger/TrigAnalysis/TriggerMatchingTool
   Tools/PathResolver
   PhysicsAnalysis/AnalysisCommon/IsolationSelection
   PhysicsAnalysis/AnalysisCommon/PileupReweighting
   DataQuality/GoodRunsLists
   PhysicsAnalysis/SUSYPhys/SUSYTools

)

# External(s) needed by the package:
find_package( ROOT COMPONENTS Core Hist Physics REQUIRED )

# Need fastjet for Sherpa parton clustering.
find_package(FastJet)

# Build a dictionary source file for the main library of the package:
atlas_add_root_dictionary( VBFInvAnalysisLib VBFInvAnalysisDictSource
   ROOT_HEADERS VBFInvAnalysis/*.h Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT )

# Build the main library of the package:
atlas_add_library( VBFInvAnalysisLib
   VBFInvAnalysis/*.h Root/*.h Root/*.cxx ${VBFInvAnalysisDictSource}
   INCLUDE_DIRS ${FASTJET_INCLUDE_DIR}
   PUBLIC_HEADERS VBFInvAnalysis
   LINK_LIBRARIES EventLoop xAODEventInfo PATInterfaces xAODJet xAODTruth xAODCore AsgAnalysisInterfaces JetInterface JetResolutionLib xAODMuon
    MuonSelectorToolsLib MuonMomentumCorrectionsLib TrigDecisionToolLib TrigConfInterfaces EventLoopAlgs PathResolver BoostedJetTaggersLib

   AthContainers CxxUtils
   xAODRootAccess xAODCore xAODEgamma xAODTau xAODJet xAODMuon xAODMissingET
   xAODEventInfo xAODTracking xAODCaloEvent xAODTruth xAODCutFlow xAODTrigger xAODMetaData
   GoodRunsListsLib PileupReweightingLib PathResolver TrigConfxAODLib TrigDecisionToolLib TriggerMatchingToolLib
   MuonMomentumCorrectionsLib FourMomUtils IsolationSelectionLib IsolationCorrectionsLib
   ElectronPhotonSelectorToolsLib TauAnalysisToolsLib ElectronEfficiencyCorrectionLib EgammaAnalysisInterfacesLib
   SUSYToolsLib GammaORToolsLib
   ${FASTJET_LIBRARIES}

   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODBase )

# Install files from the package:
atlas_install_data( share/* )
atlas_install_scripts( scripts/* )
