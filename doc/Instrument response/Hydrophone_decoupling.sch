v 20121123 2
C 40000 40000 0 0 0 title-B.sym
C 43300 45700 1 90 0 voltage-1.sym
{
T 42800 45800 5 10 0 0 90 0 1
device=VOLTAGE_SOURCE
T 42800 46000 5 10 1 1 90 0 1
refdes=Vs
}
C 45100 47700 1 270 0 capacitor-2.sym
{
T 45800 47500 5 10 0 0 270 0 1
device=POLARIZED_CAPACITOR
T 45600 47500 5 10 1 1 270 0 1
refdes=C1
T 46000 47500 5 10 0 0 270 0 1
symversion=0.1
}
C 46900 44800 1 270 0 capacitor-2.sym
{
T 47600 44600 5 10 0 0 270 0 1
device=POLARIZED_CAPACITOR
T 47400 44600 5 10 1 1 270 0 1
refdes=C2
T 47800 44600 5 10 0 0 270 0 1
symversion=0.1
}
C 45200 46500 1 270 0 resistor-1.sym
{
T 45600 46200 5 10 0 0 270 0 1
device=RESISTOR
T 45500 46300 5 10 1 1 270 0 1
refdes=R1
}
C 45200 44800 1 270 0 resistor-1.sym
{
T 45600 44500 5 10 0 0 270 0 1
device=RESISTOR
T 45500 44600 5 10 1 1 270 0 1
refdes=R2
}
C 53900 44000 1 180 0 terminal-1.sym
{
T 53590 43250 5 10 0 0 180 0 1
device=terminal
T 53590 43400 5 10 0 0 180 0 1
footprint=CONNECTOR 1 1
T 53650 43950 5 10 1 1 180 6 1
refdes=Vo-
}
C 45200 43600 1 0 0 gnd-1.sym
N 43100 45700 43100 43900 4
N 43100 43900 53000 43900 4
N 45300 44800 45300 45600 4
N 45300 46500 45300 46800 4
N 43100 48200 45300 48200 4
N 43100 46600 43100 48200 4
N 45300 48200 45300 47700 4
N 45300 45300 47100 45300 4
N 47100 44800 47100 47200 4
C 50000 46400 1 0 0 opamp-2.sym
{
T 50800 47400 5 10 0 0 0 0 1
device=OPAMP
T 50800 47200 5 10 1 1 0 0 1
refdes=OPA627
T 50800 47600 5 10 0 0 0 0 1
symversion=0.1
}
C 53900 47000 1 180 0 terminal-1.sym
{
T 53590 46250 5 10 0 0 180 0 1
device=terminal
T 53590 46400 5 10 0 0 180 0 1
footprint=CONNECTOR 1 1
T 53650 46950 5 10 1 1 180 6 1
refdes=Vo+
}
N 50000 44300 50000 46600 4
N 50000 44300 51800 44300 4
N 51800 44300 51800 46900 4
N 51100 46900 53000 46900 4
C 50600 44700 1 0 0 generic-power.sym
{
T 50800 44950 5 10 1 1 0 3 1
net=-10V
}
C 50300 48300 1 0 0 generic-power.sym
{
T 50500 48550 5 10 1 1 0 3 1
net=+10V
}
N 47100 47200 50000 47200 4
N 50500 48300 50500 47400 4
N 50500 46400 50500 44700 4
C 51300 45700 1 0 0 gnd-1.sym
C 51300 47600 1 0 0 gnd-1.sym
C 50500 47700 1 0 0 capacitor-1.sym
{
T 50700 48400 5 10 0 0 0 0 1
device=CAPACITOR
T 50700 48200 5 10 1 1 0 0 1
refdes=0.1uF
T 50700 48600 5 10 0 0 0 0 1
symversion=0.1
}
C 50500 45800 1 0 0 capacitor-1.sym
{
T 50700 46500 5 10 0 0 0 0 1
device=CAPACITOR
T 50700 46300 5 10 1 1 0 0 1
refdes=0.1uF
T 50700 46700 5 10 0 0 0 0 1
symversion=0.1
}
N 50500 44700 50800 44700 4
T 53500 44300 9 10 1 0 90 0 1
ADS1282-EVM terminals
T 43100 48200 9 10 1 0 0 0 1
+/- 6V
T 47100 47200 9 10 1 0 0 0 1
+/- 2.5V
T 52600 46900 9 10 1 0 0 0 1
+/- 2.5V
T 50100 40700 9 10 1 0 0 0 1
Hydrophone decoupling and buffer (equivalent circuit)
T 54000 40400 9 10 1 0 0 0 1
1
T 53900 40100 9 10 1 0 0 0 1
Gaute Hope <eg@gaute.vetsj.com>
T 42400 45200 9 10 1 0 90 0 1
Hydrophone signal input
