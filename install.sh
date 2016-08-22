# install.sh
# This file MUST be executed for correct installation.
# usage:
# 		install.sh /path/to/install

cmakever=
cont=
cyantxt="\033[0;36m"
redtxt="\033[0;31m"
greentxt="\033[0;32m"
endtxt="\033[0m"

echo -e $cyantxt"********************"$endtxt
echo -e $cyantxt"* Libmsr Installer *"$endtxt
echo -e $cyantxt"********************\n"$endtxt

#################
# Check for GCC #
#################
GCCVER=$(which gcc)
echo -e $cyantxt"Checking for gcc: $endtxt${GCCVER}"

if [[ $? != 0 ]]; then
	echo -e $redtxt"ERROR: gcc not found. (why don't you have gcc?)"$endtxt
fi

#######################
# Check CMake Version #
#######################
cmakever="$(cmake --version | grep -E -o [0-9][.][0-9])"
echo -e $cyantxt"CMake Version: $endtxt${cmakever}"

if [[ $cmakever < 2.8 ]]; then
	echo -e $redtxt"ERROR: cmake 2.8 or newer required\n"$endtxt
	exit 3
fi

###########################
# Detect CPU Architecture #
###########################
echo -e $cyantxt"Compiling autoconfiguration tool."$endtxt
gcc autoconf.c -o _autoconf_

if [[ $? != 0 ]]; then
	echo -e $redtxt"ERROR: could not compile autoconf file. Terminating...\n"$endtxt
	exit 4
fi

echo -e $cyantxt"Generating master header file."$endtxt
if [ $# -eq 1 ]; then
    ./_autoconf_
else
    echo $2
    ./_autoconf_ $2
fi

if [[ $? != 0 ]]; then
	echo -e $redtxt"ERROR: could not generate master header file. Terminating...\n"$endtxt
	exit 5
fi

cp platform_headers/master.h include/master.h

if [[ $? != 0 ]]; then
	echo -e $redtxt"ERROR: unable to install master header file. Terminating...\n"$endtxt
	exit 6
fi

######################################
# Congfigure CMake w/Build Directory #
######################################
# No build directory specified, so create one.
if [ -z "$1" ]; then
    if [ -e "BUILD" ]; then
        echo -e "Directory BUILD exists. overwrite? (y/n)"
        read cont
        if [ "$cont" = "y" ]; then
            rm -r BUILD
            mkdir BUILD
            if [[ $? != 0 ]]; then
                echo -e "$redtxt ERROR: could not create BUILD directory. Terminating...\n$endtxt"
                exit 2
            fi
        fi
    else
        mkdir BUILD
    fi
	echo -e $cyantxt"Configuring cmake."$endtxt
	cmake . -DCMAKE_INSTALL_PREFIX=./BUILD
# User specified build directory.
else
    mkdir $1
	echo -e $cyantxt"Configuring cmake."$endtxt
	cmake . -DCMAKE_INSTALL_PREFIX="$1"
fi

##################
# Compile Libmsr #
##################
make

if [[ $? != 0 ]]
then
	echo -e $redtxt"ERROR: unable to compile source code. Terminating...\n"$endtxt
	exit 8
fi

##################
# Install Libmsr #
##################
make install

if [[ $? != 0 ]]
then
	echo -e $redtxt"ERROR: unable to install library. Terminating...\n"$endtxt
	exit 9
fi

#####################################
# Generate HTML/Latex Documentation #
#####################################
make doc
make latex_doc

echo -e $cyantxt"Copying latex doc into documentation directory."$endtxt
raw_date=$(date)
mod_date=${raw_date// /_}
cd dox/latex && cp refman.pdf ../../documentation/doxygen_${mod_date}.pdf

############################
# Terminate Install Script #
############################
echo -e $cyantxt"Done\n"$endtxt
