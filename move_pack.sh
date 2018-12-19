#!/bin/bash

source_pack='csc501-lab2'
source_h=$source_pack'/h/'
source_sys=$source_pack'/sys/'
destination=$source_pack'/TMP'
file_changed_h=('')
file_changed_sys=('main')
file_changed_other=('./csc501-lab2/compile/Makefile')

for hfile in ${file_changed_h[*]}
do
	cp -pr $source_h$hfile'.h' $destination
done

for cfile in ${file_changed_sys[*]}
do
	cp -pr $source_sys$cfile'.c' $destination
done

for ofile in ${file_changed_other[*]}
do
	cp -pr $ofile $destination
done

tar zcvf $source_pack'.tar.gz' $source_pack
