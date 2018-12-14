#!/bin/bash

# Analysis base release
RELEASE_21="21.2.56"

echo 'Setting up VBF Ntuple Maker code in Release 21 ...'
mkdir -p build run && cd build
lsetup 'asetup AnalysisBase,${RELEASE_21},here'
cd $TestArea/..

echo 'Generating setup script...'
# Clean up if it exists..
if [ -e 'setup.sh' ]; then
  rm -rf setup.sh
fi

echo '#!/bin/bash' >> 'setup.sh'
echo "" >> "setup.sh"
echo "cd build" >> "setup.sh"
echo "lsetup \"asetup AnalysisBase,${RELEASE_21},here\" " >> "setup.sh"
#echo "source */setup.sh" >> "setup.sh"
echo "cd ../" >> "setup.sh"
echo "alias cbuild=\"cd \$TestArea/ && cmake \$TestArea/../STAnalysisCode && make && source \$TestArea/*/setup.sh && cd \$TestArea/../run\"" >> "setup.sh"
echo "alias build=\"cd \$TestArea/ && make && source \$TestArea/*/setup.sh && cd \$TestArea/.. && cd \$TestArea/../run\"" >> "setup.sh"
echo "alias clean=\"rm -rf  \$TestArea/ setup.sh\"" >> "setup.sh"
echo "echo \"Compile: cbuild, Recompile: build, Clean: clean\"" >> "setup.sh"
echo "echo \"Test with runVBF.py -f <path/file.root> -n10 --debug\"" >> "setup.sh"
echo "cd \$TestArea/.." >> "setup.sh"
echo "" >> "setup.sh"

echo "Running setup script..."
chmod +x setup.sh
source "setup.sh"
cbuild

echo ''
echo 'Finished VBF R21 setup..'
echo 'Next time you login: "source setup.sh"'
echo 'Recompile: build, Clean: clean'
echo 'Now run a quick test with "runVBF.py -f <path/file.root> -n10 --debug"'
