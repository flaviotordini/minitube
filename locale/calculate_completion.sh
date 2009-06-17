#!/bin/bash
#
# This script was written to get some data on how far the various translations are
# compared to each other
#
# This script is donated to the public domain
#
# Klaas van Gend, 2008

printf "\n   translation file  %%ready   (unfinished/(total-obsolete))\n"
printf '=============================================================\n'
for I in `ls -1 *.ts`;
do
	UNFINISHED=`grep 'type="unfinished"' $I | wc -l`;
	OBSOLETE=`grep 'obsolete' $I | wc -l`;
	MSGLINES=`grep '</message>' $I | wc -l`;
	let "REALLINES=$MSGLINES-$OBSOLETE";
	let "PERCENT=(100*$UNFINISHED)/$REALLINES";
	let "FINISHED=100-$PERCENT";
	printf "% 18s : % 4d%%    %d/(%d-%d)\n" $I $FINISHED $UNFINISHED $MSGLINES $OBSOLETE ;
done
printf "\n"

