#
# Environment configuration file setting up the installed project.
#

# Set up the base environment using the base image's setup script:
source ~/analysis_release_setup.sh

# Set up the STAnalysisCode installation:
source /usr/STAnalysisCode/*/InstallArea/*/setup.sh
echo "Configured STAnalysisCode from: $STAnalysisCode_DIR"

# Set up the prompt:
export PS1='\[\033[01;35m\][bash]\[\033[01;31m\][\u STAnalysisCode-$STAnalysisCode_VERSION]\[\033[01;34m\]:\W >\[\033[00m\] ';
