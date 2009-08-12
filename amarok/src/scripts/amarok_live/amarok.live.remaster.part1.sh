#!/usr/bin/env bash
#set -x 

############################################################################
# Disassemble an Amarok LiveCD iso file into a specified directory
# for the purpose of adding a users own music and putting it back together
#
# Based on a script authored by Ivan Kerekes <ikerekes@gmail.com>
# and modified by Leo Franchi <lfranchi at gmaildotcom>, Mike Diehl 
# <madpenguin8 at yahoodotcom> and Greg Meyer <greg at gkmwebdotcom> 
#
# See the file called README for more information
############################################################################
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
############################################################################


cleanup () {
    rm -rf $WORK/mklivecd
    rm -rf $WORK/amarok.live
    rm -rf $WORK/livecd_data*
}

# We need to be root to mount the loopback
# check for root and exit if not

if [ `/usr/bin/whoami` = 'root' ]; then

iso=$2

if [ $iso = 0 ] ; then
    exit
fi

WORK=$1

if [ $WORK = 0 ] ; then
    exit
fi

# Determine if enough space

redo=0
enough=0
tmp=$WORK
while [ "$redo" = "0" ]; do
    while [ "$enough" = "0" ]; do # loops until found something
        if [[ -n  `df | grep $tmp` ]] ; then # we got it in df, find the space left
            anotmp=`df | grep $tmp |  sed "s~^\([^ ]*\) *\([^ ]*\) *\([^ ]*\) *\([^ ]*\) *\([^ ]*\) *\([^ ]*\)$~\4~"`  # now we have a string, first item is free space in /
            free=`echo $anotmp  | sed "s~^\([^ ]*\) \(.*\)~\1~"` # get first space-delimited item
#			echo "comparing" $free "to 1572864"
            if [[ $free -gt 1572864 ]] ; then
                enough=1
                redo=1
                break
            else
                dcop --all-users amarok playlist popupMessage "Not enough free space. Please select another folder."
                break
            fi
        else
            res=`echo "$tmp" | sed "s~/\(.*\)/\([^/]*\)$~/\1~"`
            if [[ "$tmp" = "$res" ]]; then # last one, regexp fails here
                tmp="/"
            else #normal, removes one dir from path
                tmp=$res
            fi
#            echo "new tmp: " $tmp
        fi
    done
    if [[ "$redo" = "0" ]]; then
        WORK=`kdialog --title "Choose working directory" --getexistingdirectory .`
		tmp=$WORK
		if [[ "$?" == 0 ]]; then
			break
        else
           exit 
		fi
		enough=0
    fi
done

#Mount the iso if not already mounted
if [ ! -d "$DATADIR" ]; then
   DATADIR=$WORK/livecd_data$$
   mkdir -p "$DATADIR"
   mount -o loop "$iso" "$DATADIR"
fi

# Make the working directories and blow out the initrd.gz into a separate directory 
 
mkdir -p $WORK/mklivecd/livecd
cp -a --preserve "$DATADIR"/* $WORK/mklivecd/livecd/
mkdir -p $WORK/mklivecd/initrd.dir
mkdir -p $WORK/mklivecd/initrd.mnt
gunzip -c $WORK/mklivecd/livecd/isolinux/initrd.gz > $WORK/mklivecd/livecd/isolinux/initrd
mount -o loop $WORK/mklivecd/livecd/isolinux/initrd $WORK/mklivecd/initrd.mnt
(cd $WORK/mklivecd/initrd.mnt ; tar cf - .) | (cd $WORK/mklivecd/initrd.dir ; tar xf -)
umount $WORK/mklivecd/initrd.mnt
rm -f $WORK/mklivecd/livecd/isolinux/initrd


# cleanup all temporary files and directories

umount "$DATADIR" 2>/dev/null >/dev/null
if [ "$?" = "0" ]; then rmdir $DATADIR; fi

# at this point we unsquash the fs so the user can add their own music

if [[  `cat /proc/filesystems | grep squash | wc -l` = 0 ]]; then
	modprobe squashfs
	if [[  `cat /proc/filesystems | grep squash | wc -l` = 0 ]]; then

    	 dcop --all-users amarok playlist popupMessage "You do not have squashfs support enabled. You need to have a patched kernel with squashfs. You can find more info about squashfs, and how to patch your kernel, here: http://tldp.org/HOWTO/SquashFS-HOWTO/"
	
		rm -rf $WORK/mklivecd
	fi
fi

mkdir $WORK/amarok.livecd/
mount -o loop -t squashfs $WORK/mklivecd/livecd/livecd.sqfs $WORK/amarok.livecd/

# gotta copy it locally so the user can add files to it

mkdir $WORK/amarok.live/
dcop --all-users amarok playlist shortStatusMessage "Copying files now. Please be patient, this step takes a long time."
#echo
#echo "Please wait, copying in progress."
#echo
cp -a $WORK/amarok.livecd/* $WORK/amarok.live/
umount $WORK/amarok.livecd/
rmdir $WORK/amarok.livecd

mkdir $WORK/amarok.live/home/amarok/.kde/share/apps/amarok/playlists
chown 500:500 $WORK/amarok.live/home/amarok/.kde/share/apps/amarok/playlists
find $WORK/amarok.live/home/amarok/.kde/share/apps/ -type d -print0 | xargs -0 chmod +x
chmod -R 777 $WORK/amarok.live/music/
chmod -R 777 $WORK/amarok.live/home/amarok/.kde/share/apps/amarok
#chmod -R 777 $WORK/mklivecd/

dcop --all-users amarok playlist popupMessage "Copying done. To add music to the Amarok livecd, select the tracks you wish to add in the playlist, and select \"Add to livecd\" from the right click menu. Please do not add more than about 380 mb, as then the resulting ISO will be too large to fit on a CD-ROM. Once you are done, select Create Remastered CD. Enjoy!"

else 

kdialog --title "Amarok livecd remaster" --sorry "You must run this script as root. Try running 'kdesu sh amarok.live.remaster.part1.sh' instead."

fi
