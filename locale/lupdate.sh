#!/bin/bash
#
# This script was written to update all the .ts files in one go
#
# This script is donated to the public domain
#
# Flavio Tordini, 2009

for I in `ls -1 *.ts`;
do
  echo Updating $I
  lupdate-qt4 ../minitube.pro -ts $I
done

