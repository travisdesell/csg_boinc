#!/bin/csh

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2013 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

##
# Release Script for Macintosh BOINC Manager 10/31/07 by Charlie Fenton
## updated 11/18/09 by Charlie Fenton for OS 10.6 Snow Leopard
## updated 9/28/10 by Charlie Fenton for new BOINC skins
## updated 12/2/10 by Charlie Fenton to remove obsolete items
## updated 1/18/11 by Charlie Fenton to remove BOINC skins
## updated 11/9/11 by Charlie Fenton for XCode 4.1 and OS 10.7
## updated 11/26/11 by Charlie Fenton for new Default skin
## updated 11/27/11 by Charlie Fenton for new virtualbox directory
## updated 12/2/11 by Charlie Fenton to restore wrapper and reboot if needed
## updated 1/6/12 by Charlie Fenton to also install VirtualBox
## updated 6/22/12 by Charlie Fenton to code sign the installer and uninstaller
## updated 7/5/12 by Charlie Fenton to avoid using PackageMaker
## updated 7/31/12 by Charlie Fenton for Liberation font in boincscr
## updated 6/11/13 by Charlie Fenton for BOINC.mpkg, "BOINC + VirtualBox.mpkg"
## updated 6/18/13 by Charlie Fenton for localizable uninstaller
## updated 8/15/13 by Charlie Fenton to fix bug in localizable uninstaller
## updated 10/30/13 by Charlie Fenton to build a flat package
## updated 11/1/13 by Charlie Fenton to build installers both with and without VBox
## updated 11/18/13 by Charlie Fenton for Xcode 5.0.2
## updated 1/22/14 by Charlie Fenton: embed VBox uninstaller in BOINC uninstaller
##
## NOTE: This script requires Mac OS 10.6 or later, and uses XCode developer
##   tools.  So you must have installed XCode Developer Tools on the Mac 
##   before running this script.
##

## NOTE: To build the executables under Lion and XCode 4, select from XCode's
## menu: "Product/Buildfor/Build for Archiving", NOT "Product/Archive"

## To have this script build the combined BOINC+VirtualBox installer:
## * Create a directory named "VirtualBox Installer" in the same
##   directory which contains he root directory of the boinc tree.
## * Copy VirtualBox.pkg from the VirtualBox installer disk image (.dmg)
##   into this "VirtualBox Installer" directory.
## * Copy VirtualBox_Uninstall.tool from the VirtualBox installer disk
##   image (.dmg) into this "VirtualBox Installer" directory.
##

## Usage:
##
## If you wish to code sign the installer and uninstaller, create a file 
## ~/BOINCCodeSignIdentity.txt whose first line is the code signing identity
##
## cd to the root directory of the boinc tree, for example:
##     cd [path]/boinc
##
## Invoke this script with the three parts of version number as arguments.  
## For example, if the version is 3.2.1:
##     source [path_to_this_script] 3 2 1
##
## This will create a director "BOINC_Installer" in the parent directory of 
## the current directory
##
## For testing only, you can use the development build by adding a fourth argument -dev
## For example, if the version is 3.2.1:
##     source [path_to_this_script] 3 2 1 -dev

if [ $# -lt 3 ]; then
echo "Usage:"
echo "   cd [path]/boinc"
echo "   source [path_to_this_script] major_version minor_version revision_number"
return 1
fi

#pushd ./
BOINCPath=$PWD

DarwinVersion=`uname -r`;
DarwinMajorVersion=`echo $DarwinVersion | sed 's/\([0-9]*\)[.].*/\1/' `;
# DarwinMinorVersion=`echo $version | sed 's/[0-9]*[.]\([0-9]*\).*/\1/' `;
#
# echo "major = $DarwinMajorVersion"
# echo "minor = $DarwinMinorVersion"
#
# Darwin version 11.x.y corresponds to OS 10.7.x
# Darwin version 10.x.y corresponds to OS 10.6.x
# Darwin version 8.x.y corresponds to OS 10.4.x
# Darwin version 7.x.y corresponds to OS 10.3.x
# Darwin version 6.x corresponds to OS 10.2.x

if [ "$DarwinMajorVersion" -gt 10 ]; then
    # XCode 4.1 on OS 10.7 builds only Intel binaries
    arch="i686"

    # XCode 3.x and 4.x use different paths for their build products.
    # Our scripts in XCode's script build phase write those paths to 
    # files to help this release script find the build products.
    if [ "$4" = "-dev" ]; then
        exec 7<"mac_build/Build_Development_Dir"
        read -u 7 BUILDPATH
    else
        exec 7<"mac_build/Build_Deployment_Dir"
        read -u 7 BUILDPATH
    fi

else
    # XCode 3.2 on OS 10.6 does sbuild Intel and PowerPC Universal binaries
    arch="universal"

    # XCode 3.x and 4.x use different paths for their build products.
    if [ "$4" = "-dev" ]; then
        if [ -d mac_build/build/Development/ ]; then
            BUILDPATH="mac_build/build/Development"
        else
            BUILDPATH="mac_build/build"
        fi
    else
        if [ -d mac_build/build/Deployment/ ]; then
            BUILDPATH="mac_build/build/Deployment"
        else
            BUILDPATH="mac_build/build"
        fi
    fi
fi

sudo rm -dfR ../BOINC_Installer/Installer\ Resources/
sudo rm -dfR ../BOINC_Installer/Installer\ Scripts/
sudo rm -dfR ../BOINC_Installer/Pkg_Root
sudo rm -dfR ../BOINC_Installer/locale
sudo rm -dfR ../BOINC_Installer/Installer\ templates
sudo rm -dfR ../BOINC_Installer/expandedVBox

mkdir -p ../BOINC_Installer/Installer\ Resources/
mkdir -p ../BOINC_Installer/Installer\ Scripts/
mkdir -p ../BOINC_Installer/Installer\ templates

cp -fp mac_installer/License.rtf ../BOINC_Installer/Installer\ Resources/
cp -fp mac_installer/ReadMe.rtf ../BOINC_Installer/Installer\ Resources/

cp -fp mac_installer/complist.plist ../BOINC_Installer/Installer\ templates
cp -fp mac_installer/myDistribution ../BOINC_Installer/Installer\ templates

# Update version number
sed -i "" s/"<VER_NUM>"/"$1.$2.$3"/g ../BOINC_Installer/Installer\ Resources/ReadMe.rtf
sed -i "" s/"x.y.z"/"$1.$2.$3"/g ../BOINC_Installer/Installer\ templates/myDistribution

#### We don't customize BOINC Data directory name for branding
cp -fp mac_installer/preinstall ../BOINC_Installer/Installer\ Scripts/
cp -fp mac_installer/preinstall ../BOINC_Installer/Installer\ Scripts/preupgrade
cp -fp mac_installer/postinstall ../BOINC_Installer/Installer\ Scripts/
cp -fp mac_installer/postupgrade ../BOINC_Installer/Installer\ Scripts/

mkdir -p ../BOINC_Installer/Pkg_Root
mkdir -p ../BOINC_Installer/Pkg_Root/Applications
mkdir -p ../BOINC_Installer/Pkg_Root/Library
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/skins
mkdir -p ../BOINC_Installer/Pkg_Root/tmp

# We must create virtualbox directory so installer will set up its
# ownership and permissions correctly, because vboxwrapper won't 
# have permission to set owner to boinc_master.
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/projects/virtualbox

##cp -fpR $BUILDPATH/WaitPermissions.app ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/

cp -fpRL $BUILDPATH/switcher ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/
cp -fpRL $BUILDPATH/setprojectgrp ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/
## cp -fpRL $BUILDPATH/AppStats ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/

cd "${BOINCPath}/clientgui/skins"
cp -fpRL Default ../../../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/skins/
cd "${BOINCPath}"

cp -fp curl/ca-bundle.crt ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/

cp -fp win_build/installerv2/redist/all_projects_list.xml ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/installer_projects_list.xml

cp -fp doc/logo/boinc_logo_black.jpg ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fp api/ttf/liberation-fonts-ttf-2.00.0/LiberationSans-Regular.ttf ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/LiberationSans-Regular.ttf
cp -fp clientscr/ss_config.xml ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL $BUILDPATH/boincscr ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/

cp -fpRL $BUILDPATH/BOINCManager.app ../BOINC_Installer/Pkg_Root/Applications/

cp -fpRL $BUILDPATH/BOINCSaver.saver ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers/

cp -fpRL $BUILDPATH/PostInstall.app ../BOINC_Installer/Pkg_Root/tmp/

## Copy the localization files into the installer tree
## Old way copies CVS and *.po files which are not needed
## cp -fpRL locale/ ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale
## sudo rm -dfR ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale/CVS
## New way copies only *.mo files (adapted from boinc/sea/make-tar.sh)
find locale -name '*.mo' | cut -d '/' -f 2 | awk '{print "\"../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/locale/"$0"\""}' | xargs mkdir -p
find locale -name '*.mo' | cut -d '/' -f 2,3 | awk '{print "cp \"locale/"$0"\" \"../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/locale/"$0"\""}' | bash

## Fix up ownership and permissions
sudo chown -R root:admin ../BOINC_Installer/Pkg_Root/*
sudo chmod -R u+rw,g+rw,o+r-w ../BOINC_Installer/Pkg_Root/*
sudo chmod 1775 ../BOINC_Installer/Pkg_Root/Library

sudo chown -R 501:admin ../BOINC_Installer/Pkg_Root/Library/Application\ Support/*
sudo chmod -R u+rw,g+r-w,o+r-w ../BOINC_Installer/Pkg_Root/Library/Application\ Support/*

sudo chown -R root:admin ../BOINC_Installer/Installer\ Resources/*
sudo chown -R root:admin ../BOINC_Installer/Installer\ Scripts/*
sudo chmod -R u+rw,g+r-w,o+r-w ../BOINC_Installer/Installer\ Resources/*
sudo chmod -R u+rw,g+r-w,o+r-w ../BOINC_Installer/Installer\ Scripts/*

sudo rm -dfR ../BOINC_Installer/New_Release_$1_$2_$3/

mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_SymbolTables

cp -fp ../BOINC_Installer/Installer\ Resources/ReadMe.rtf ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/ReadMe.rtf
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/ReadMe.rtf

cp -fp COPYING ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.txt
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.txt
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.txt

cp -fp COPYING.LESSER ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.LESSER.txt
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.LESSER.txt
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.LESSER.txt

cp -fp COPYRIGHT ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYRIGHT.txt
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYRIGHT.txt
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYRIGHT.txt

cp -fpRL $BUILDPATH/Uninstall\ BOINC.app ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras
# Copy the localization files for the uninstaller into its bundle
find locale -name 'BOINC-Setup.mo' | cut -d '/' -f 2 | awk '{print "\"../BOINC_Installer/locale/"$0"\""}' | xargs mkdir -p

find locale -name 'BOINC-Setup.mo' | cut -d '/' -f 2,3 | awk '{print "cp \"locale/"$0"\" \"../BOINC_Installer/locale/"$0"\""}' | bash

sudo cp -fpRL ../BOINC_Installer/locale "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources"

sudo chown -R root:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall\ BOINC.app
sudo chmod -R u+r-w,g+r-w,o+r-w ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall\ BOINC.app

# Copy the installer wrapper application "BOINC Installer.app"
cp -fpRL $BUILDPATH/BOINC\ Installer.app ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/

# Prepare to build the BOINC+VirtualBox installer if VirtualBox.pkg exists
VirtualBoxPackageName="VirtualBox.pkg"
if [ -f "../VirtualBox Installer/${VirtualBoxPackageName}" ]; then
    # Make a copy of the  BOINC installer app without the installer package
    sudo cp -fpRL "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox"

    # Copy the VirtualBox uninstall tool into the extras directory
    sudo cp -fpRL "../VirtualBox Installer/VirtualBox_Uninstall.tool" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/"

    sudo chown -R root:admin "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/VirtualBox_Uninstall.tool"
    sudo chmod -R u+r-w,g+r-w,o+r-w "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/VirtualBox_Uninstall.tool"

    # Copy the VirtualBox uninstall tool into the BOINC uninstaller
    sudo cp -fpRL "../VirtualBox Installer/VirtualBox_Uninstall.tool" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/Uninstall BOINC.app/Contents/Resources"

    sudo chown -R root:admin "../VirtualBox Installer/VirtualBox_Uninstall.tool" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/Uninstall BOINC.app/Contents/Resources/VirtualBox_Uninstall.tool"
    sudo chmod -R u+r-w,g+r-w,o+r-w "../VirtualBox Installer/VirtualBox_Uninstall.tool" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/Uninstall BOINC.app/Contents/Resources/VirtualBox_Uninstall.tool"
fi

# Build the installer package inside the wrapper application's bundle

cd "../BOINC_Installer/Installer templates"

pkgbuild --quiet --scripts "../Installer Scripts" --ownership recommended --identifier edu.berkeley.boinc --root "../Pkg_Root" --component-plist "./complist.plist" "./BOINC.pkg"

productbuild --quiet --resources "../Installer Resources/" --version "BOINC Manager $1.$2.$3" --distribution "./myDistribution" "../New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/BOINC Installer.app/Contents/Resources/BOINC.pkg"

cd "${BOINCPath}"

# Build the BOINC+VirtualBox installer if VirtualBox.pkg exists
if [ -f "../VirtualBox Installer/${VirtualBoxPackageName}" ]; then
    cp -fpRL "mac_installer/V+BDistribution" "../BOINC_Installer/Installer templates/V+BDistribution"

    mkdir -p "../BOINC_Installer/expandedVBox/"

    pkgutil --expand "../VirtualBox Installer/${VirtualBoxPackageName}" "../BOINC_Installer/expandedVBox/VBox.pkg"

    pkgutil --flatten "../BOINC_Installer/expandedVBox/VBox.pkg/VBoxKEXTs.pkg" "../BOINC_Installer/Installer templates/VBoxKEXTs.pkg"

    pkgutil --flatten "../BOINC_Installer/expandedVBox/VBox.pkg/VBoxStartupItems.pkg" "../BOINC_Installer/Installer templates/VBoxStartupItems.pkg"

    pkgutil --flatten "../BOINC_Installer/expandedVBox/VBox.pkg/VirtualBox.pkg" "../BOINC_Installer/Installer templates/VirtualBox.pkg"

    pkgutil --flatten "../BOINC_Installer/expandedVBox/VBox.pkg/VirtualBoxCLI.pkg" "../BOINC_Installer/Installer templates/VirtualBoxCLI.pkg"

    cp -fpRL "../BOINC_Installer/expandedVBox/VBox.pkg/Resources/en.lproj" "../BOINC_Installer/Installer Resources"

    sudo rm -dfR "../BOINC_Installer/expandedVBox"

    cp -fp mac_installer/V+BDistribution "../BOINC_Installer/Installer templates"

    # Update version number
    sed -i "" s/"x.y.z"/"$1.$2.$3"/g "../BOINC_Installer/Installer templates/V+BDistribution"

    cd "../BOINC_Installer/Installer templates"

    productbuild --quiet --resources "../Installer Resources" --version "BOINC Manager 7.3.0 + VirtualBox 4.2.16" --distribution "./V+BDistribution" "../New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/BOINC Installer.app/Contents/Resources/BOINC.pkg"

    cd "${BOINCPath}"
fi

# Build the stand-alone client distribution
cp -fpRL mac_build/Mac_SA_Insecure.sh ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/
cp -fpRL mac_build/Mac_SA_Secure.sh ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/
cp -fpRL COPYING ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/COPYING.txt
cp -fpRL COPYING.LESSER ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/COPYING.LESSER.txt
cp -fpRL COPYRIGHT ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/COPYRIGHT.txt
cp -fp mac_installer/License.rtf ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/*
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/*

mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir
cp -fpRL $BUILDPATH/boinc ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/
cp -fpRL $BUILDPATH/boinccmd ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/
cp -fpRL curl/ca-bundle.crt ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/

mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher
cp -fpRL $BUILDPATH/switcher ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/
cp -fpRL $BUILDPATH/setprojectgrp ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/

sudo chown -R root:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/*
sudo chmod -R u+rw-s,g+r-ws,o+r-w ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/*

cp -fpRL $BUILDPATH/SymbolTables/ ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_SymbolTables/

## If you wish to code sign the installer and uninstaller, create a file 
## ~/BOINCCodeSignIdentity.txt whose first line is the code signing identity
##
## Code signing using a registered Apple Developer ID is necessary for GateKeeper 
## with default settings to allow running downloaded applications under OS 10.8
if [ -e "${HOME}/BOINCCodeSignIdentity.txt" ]; then
    exec 8<"${HOME}/BOINCCodeSignIdentity.txt"
    read -u 8 SIGNINGIDENTITY

    # Code Sign the BOINC installer if we have a signing identity
    sudo codesign -f -s "${SIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/BOINC Installer.app"

    # Code Sign the BOINC uninstaller if we have a signing identity
    sudo codesign -f -s "${SIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app"
fi

cd ../BOINC_Installer/New_Release_$1_$2_$3
## Use ditto instead of zip utility to preserve resource forks and Finder attributes (custom icon, hide extension) 
ditto -ck --sequesterRsrc --keepParent boinc_$1.$2.$3_macOSX_$arch boinc_$1.$2.$3_macOSX_$arch.zip
ditto -ck --sequesterRsrc --keepParent boinc_$1.$2.$3_$arch-apple-darwin boinc_$1.$2.$3_$arch-apple-darwin.zip
ditto -ck --sequesterRsrc --keepParent boinc_$1.$2.$3_macOSX_SymbolTables boinc_$1.$2.$3_macOSX_SymbolTables.zip
if [ -d boinc_$1.$2.$3_macOSX_${arch}_vbox ]; then
    ditto -ck --sequesterRsrc --keepParent boinc_$1.$2.$3_macOSX_${arch}_vbox boinc_$1.$2.$3_macOSX_${arch}_vbox.zip
fi

#popd
cd "${BOINCPath}"

sudo rm -dfR ../BOINC_Installer/Installer\ Resources/
sudo rm -dfR ../BOINC_Installer/Installer\ Scripts/
sudo rm -dfR ../BOINC_Installer/Pkg_Root
sudo rm -dfR ../BOINC_Installer/locale
sudo rm -dfR ../BOINC_Installer/Installer\ templates
sudo rm -dfR ../BOINC_Installer/expandedVBox

return 0
