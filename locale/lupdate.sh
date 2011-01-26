#!/bin/bash
for I in `ls -1 *.ts`;
do
  echo Updating $I
  lupdate ../minitube.pro -ts $I
done

