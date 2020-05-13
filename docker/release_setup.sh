#
# Environment configuration file setting up the installed project.
#

# Set up the base environment using the base image's setup script:
source  /home/atlas/analysis_release_setup.sh

# Set up the STAnalysisCode installation:
#source /usr/STAnalysisCode/*/InstallArea/*/setup.sh
source /usr/WorkDir/*/InstallArea/*/setup.sh
echo "Configured STAnalysisCode from: $WorkDir_DIR"

# Set up the prompt:
export PS1='\[\033[01;35m\][bash]\[\033[01;31m\][\u STAnalysisCode-$STAnalysisCode_VERSION]\[\033[01;34m\]:\W >\[\033[00m\] ';
