#! /bin/bash

# re create hypocenter jobs, solve and plot

makehcjob.py -j 01
makehcjob.py -j 02
makehcjob.py -j 03
makehcjob.py -j 04

cd job_hc01; hyp *.S*; cd ..
cd job_hc02; hyp *.S*; cd ..
cd job_hc03; hyp *.S*; cd ..
cd job_hc04; hyp *.S*; cd ..

plotevent_ibcao.py

