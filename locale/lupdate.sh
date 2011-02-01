#!/bin/bash
for I in `ls -1 *.ts`;
do
  echo Updating $I
  lupdate ../minitube.pro -codecfortr UTF-8 -ts $I
done

