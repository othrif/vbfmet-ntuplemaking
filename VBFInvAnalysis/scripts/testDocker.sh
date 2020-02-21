#echo "Checking build dir"
#pwd
#ls ../*

##############################
# Setup                      #
##############################
#echo "SETUP Release"
#source ~/release_setup.sh
#echo "SETUP Executables"
#source ../build/${AnalysisBase_PLATFORM}/setup.sh

# DAOD file types to loop over
TEST_TYPES=(Signal)

# Remote DAOD filenames
TESTFILE_ORIGINS=("root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/phys-exotics/jdm/vbfinv/RECAST/input/DAOD_EXOT5/mc16_13TeV.346600.PowhegPy8EG_NNPDF30_AZNLOCTEQ6L1_VBFH125_ZZ4nu_MET75.deriv.DAOD_EXOT5.e7613_s3126_r9364_p3895/DAOD_EXOT5.18795494._000010.pool.root.1")

# Local names for the DAODs
TESTFILE_LOCALS=(test_signal.root)

# Directory names to loop over
TEST_DIRS=(signal_test data_test)                    # Directory for results
TESTFILE_DIRS=(TestFiles_signal)      # Directory to contain test files
TESTFILE_SUBDIRS=(mc16_13TeV.signal)   # Sub-directory to contain test files (need two directory levels due to the way that monojetSubmit.py looks for the files)
OUTPUT_DIRS=(testOutput_signal)      # Directory to contain the output minitrees

i_type=0

while [  $i_type -lt 1 ]; do

  echo "RUNNING TEST OF NTUPLING : " "${TEST_TYPES[$i_type]}"

  ##############################
  # Process test sample        #
  ##############################

  # create directory for results
  pwd
  mkdir -p "${TEST_DIRS[$i_type]}"
  cd "${TEST_DIRS[$i_type]}"
  pwd
  # copy file with xrdcp
  if [ ! -f "${TESTFILE_LOCALS[$i_type]}" ]; then
      echo "File not found! Copying it from EOS"
      # get kerberos token with service account  to access central test samples on EOS
      echo "Setting up kerberos"
      echo "CERN_USER - "${KRB_USERNAME}
      echo "SERVICE_PASS - "${KRB_PASSWORD}
      echo ${KRB_PASSWORD} | kinit ${KRB_USERNAME} #@CERN.CH

      echo xrdcp "${TESTFILE_ORIGINS[$i_type]}" "${TESTFILE_LOCALS[$i_type]}"
      xrdcp "${TESTFILE_ORIGINS[$i_type]}" "${TESTFILE_LOCALS[$i_type]}"
  fi

  # run a test job
  echo "Running a test job"
  echo "Current Dir : "
  pwd
  echo "Place where executable lives : "
  #ls ../source/*
  echo "Running job now : "
  echo "runVBF.py -f ${TESTFILE_LOCALS[$i_type]}"
  ls *
  runVBF.py -f "${TESTFILE_LOCALS[$i_type]}"

  # Increment counter
  i_type=$(( $i_type + 1))

  # cd back
  cd ..

done

echo "DONE"

