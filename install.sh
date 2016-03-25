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

echo -e "$cyantxt********************\n  Libmsr Installer  \n********************\n$endtxt"

echo -e "checking for gcc"
which gcc

if [[ $? != 0 ]]
then
	echo -e "$redtxt ERROR: gcc not found. (why don't you have gcc?)$endtxt"
fi

cmakever="$(cmake --version | grep -E -o [0-9][.][0-9])"

if [[ $cmakever < 2.8 ]]
then
	echo -e "$redtxt ERROR: cmake 2.8 or newer required\n$endtxt"
	exit 3
fi

echo -e "compiling autoconfiguration tool"
gcc autoconf.c -o _autoconf_

if [[ $? != 0 ]]
then
	echo -e "$redtxt ERROR: could not compile autoconf file. Terminating...\n$endtxt"
	exit 4
fi

echo -e "generating master header file"
./_autoconf_ $2

if [[ $? != 0 ]]
then
	echo -e "$redtxt ERROR: could not generate master header file. Terminating...\n$endtxt"
	exit 5
fi

cp headers/master.h include/master.h

if [[ $? != 0 ]]
then
	echo -e "$redtxt ERROR: unable install master header file. Terminating...\n$endtxt"
	exit 6
fi

if [ -e "BUILD" ]
then
	echo -e "directory BUILD exists. overwrite? (y/n)"
	read cont
	if [ "$cont" = "y" ]
	then
		rm -r BUILD
		mkdir BUILD
		if [[ $? != 0 ]]
		then
			echo -e "$redtxt ERROR: could not create BUILD directory. Terminating...\n$endtxt"
			exit 2
		fi
	fi
else
	mkdir BUILD
fi

cd BUILD

if [[ $? != 0 ]]
then
	echo -e "$redtxt ERROR: could not enter build directory. Terminating...\n$endtxt"
	exit 7
fi

if [ "$cont" != "n" ]
then
	echo -e "configuring cmake"
	cmake .. -DCMAKE_INSTALL_PREFIX="$1"
fi

echo -e "compiling source"
make

if [[ $? != 0 ]]
then
	echo -e "$redtxt ERROR: unable to compile source code. Terminating...\n$endtxt"
	exit 8
fi

echo -e "installing" 
make install

if [[ $? != 0 ]]
then
	echo -e "$redtxt ERROR: unable to install library. Terminating...\n$endtxt"
	exit 9
fi

cd -
echo -e "$cyantxt done\n$endtxt"
