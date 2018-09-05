display_usage() {
    echo "####################### Usage ###########################"
    echo "Submission script for the VBF Higgs to Invisible Analysis"
    echo "Create arguments in: data/arguments/args_${version}.txt"
    echo "Create list in: data/samples/${version}_${desc}.txt"
    echo "runVBF.sh <version> <DAOD_Description>"
    echo "i.e: runVBF.sh v05 pilot"
    echo "If Grid tools are not setup run: grid"
    echo "#####################################################"
}

# if less than one argument supplied, display usage
if [  $# -le 1 ]
    then
    display_usage
    exit 1
    fi
# check whether user had supplied -h or --help . If yes display usage
if [[ ( $# == "--help") ||  $# == "-h" ]]
    then
    display_usage
    exit 0
    fi

version=$1
desc=$2
path="$TestArea/../STAnalysisCode/VBFInvAnalysis/data"
args_file="$path/arguments/${version}_args.txt"
list_file="$path/samples/${version}_${desc}.txt"
log_file="./${version}_${desc}.log"

echo "Make sure files needed for the submission exist..."
if [ -f "$args_file" ]
then
    echo "$args_file found."
else
    echo "$args_file not found. Aborting..."
    exit 0
fi
if [ -f "$list_file" ]
then
    echo "$list_file found."
else
    echo "$list_file not found. Aborting..."
    exit 0
fi
if [ -f "$log_file" ]
then
    echo "$log_file exists already. Overwrite ? y/n "
    read ANSWER
    case $ANSWER in
	[yY]) rm -f "$log_file" ;;
	[nN]) now=`date +%Y%m%d%H%M%S`; log_file="./logProduction/${version}/log_${desc}_${now}.txt"; ;;
    esac
else
    echo "Will create a log file: ${log_file}"
fi

echo "Done checking, all files needed for the submission exist."

runVBF.py @${args_file} --inputRucioLists=${list_file} -s submitDir_${version}_${desc} 2>&1 | tee ${log_file}



