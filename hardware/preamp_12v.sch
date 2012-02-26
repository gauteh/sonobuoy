v 20110115 2
C 40000 40000 0 0 0 title-B.sym
C 41400 49400 1 0 0 connector2-1.sym
{
T 41600 50400 5 10 0 0 0 0 1
device=CONNECTOR_2
T 41400 50200 5 10 1 1 0 0 1
refdes=Power
}
C 46300 47900 1 0 0 lm7806-1.sym
{
T 47900 49200 5 10 0 0 0 0 1
device=7806
T 47700 48900 5 10 1 1 0 6 1
refdes=TLE2426
}
N 43100 49900 54800 49900 4
N 46300 49900 46300 48500 4
N 43100 49600 43100 46300 4
N 43100 46300 54800 46300 4
N 47100 46300 47100 47900 4
C 49700 46800 1 0 0 opamp-1.sym
{
T 50400 47600 5 10 0 0 0 0 1
device=OPAMP
T 50400 47400 5 10 1 1 0 0 1
refdes=U?
T 50400 48200 5 10 0 0 0 0 1
symversion=0.1
}
C 48400 48400 1 0 0 resistor-2.sym
{
T 48800 48750 5 10 0 0 0 0 1
device=RESISTOR
T 48600 48700 5 10 1 1 0 0 1
refdes=41.67kOhm
}
C 50500 48400 1 0 0 resistor-2.sym
{
T 50900 48750 5 10 0 0 0 0 1
device=RESISTOR
T 50700 48700 5 10 1 1 0 0 1
refdes=58.33kOhm
}
C 51700 48300 1 0 0 capacitor-1.sym
{
T 51900 49000 5 10 0 0 0 0 1
device=CAPACITOR
T 51900 48800 5 10 1 1 0 0 1
refdes=47uF
T 51900 49200 5 10 0 0 0 0 1
symversion=0.1
}
C 52900 49900 1 270 0 capacitor-2.sym
{
T 53600 49700 5 10 0 0 270 0 1
device=POLARIZED_CAPACITOR
T 53400 49700 5 10 1 1 270 0 1
refdes=100uF
T 53800 49700 5 10 0 0 270 0 1
symversion=0.1
}
N 47900 48500 48400 48500 4
N 49300 48500 50500 48500 4
N 51400 48500 51700 48500 4
N 53100 49000 53100 46300 4
N 50200 46800 50200 46300 4
N 50700 47200 51400 47200 4
N 51400 47200 51400 45100 4
N 51400 46600 49500 46600 4
N 49500 46600 49500 47000 4
N 49500 47000 49700 47000 4
N 49600 48500 49600 47400 4
N 49600 47400 49700 47400 4
N 50200 47600 50200 49900 4
C 55700 50000 1 180 0 terminal-1.sym
{
T 55390 49250 5 10 0 0 180 0 1
device=terminal
T 55390 49400 5 10 0 0 180 0 1
footprint=CONNECTOR 1 1
T 55450 49950 5 10 1 1 180 6 1
refdes=Hyd (+12V)
}
C 55700 48600 1 180 0 terminal-1.sym
{
T 55390 47850 5 10 0 0 180 0 1
device=terminal
T 55390 48000 5 10 0 0 180 0 1
footprint=CONNECTOR 1 1
T 55450 48550 5 10 1 1 180 6 1
refdes=Hyd (signal)
}
C 55700 46400 1 180 0 terminal-1.sym
{
T 55390 45650 5 10 0 0 180 0 1
device=terminal
T 55390 45800 5 10 0 0 180 0 1
footprint=CONNECTOR 1 1
T 55450 46350 5 10 1 1 180 6 1
refdes=Hyd (GND)
}
C 53100 45300 1 180 0 connector2-1.sym
{
T 52900 44300 5 10 0 0 180 0 1
device=CONNECTOR_2
T 53100 44500 5 10 1 1 180 0 1
refdes=AD
}
N 54800 48500 52600 48500 4
N 51400 44800 48100 44800 4
N 48100 44800 48100 48500 4
C 48400 47500 1 0 0 capacitor-1.sym
{
T 48600 48200 5 10 0 0 0 0 1
device=CAPACITOR
T 48600 48000 5 10 1 1 0 0 1
refdes=381pF
T 48600 48400 5 10 0 0 0 0 1
symversion=0.1
}
N 48400 47700 48100 47700 4
N 49300 47700 49600 47700 4
C 43900 49900 1 0 0 12V-plus-1.sym
C 44100 46000 1 0 0 gnd-1.sym
C 47900 48500 1 0 0 generic-power.sym
{
T 48100 48750 5 10 1 1 0 3 1
net=+6V
}
T 50100 40700 9 10 1 0 0 0 1
Pre-amplifier with hydrophone decoupling and ADC buffer
T 54000 40100 9 10 1 0 0 0 1
G. Hope
