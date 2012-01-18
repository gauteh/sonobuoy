v 20110115 2
C 40000 40000 0 0 0 title-B.sym
C 43400 47500 1 90 0 voltage-1.sym
{
T 42900 47600 5 10 0 0 90 0 1
device=VOLTAGE_SOURCE
T 42900 47800 5 10 1 1 90 0 1
refdes=6 (+/- 6)V
}
C 45600 48300 1 90 0 capacitor-1.sym
{
T 44900 48500 5 10 0 0 90 0 1
device=CAPACITOR
T 45100 48500 5 10 1 1 90 0 1
refdes=47uF
T 44700 48500 5 10 0 0 90 0 1
symversion=0.1
}
N 43200 48400 43200 49200 4
N 43200 49200 45400 49200 4
N 43200 44700 43200 47500 4
C 55200 46800 1 180 0 terminal-1.sym
{
T 54890 46050 5 10 0 0 180 0 1
device=terminal
T 54890 46200 5 10 0 0 180 0 1
footprint=CONNECTOR 1 1
T 54950 46750 5 10 1 1 180 6 1
refdes=A +
}
C 45500 47000 1 90 0 resistor-2.sym
{
T 45150 47400 5 10 0 0 90 0 1
device=RESISTOR
T 45200 47200 5 10 1 1 90 0 1
refdes=R2
T 45500 47000 5 10 1 1 0 0 1
value=58,33k
}
N 40800 44700 54300 44700 4
C 55200 44800 1 180 0 terminal-1.sym
{
T 54890 44050 5 10 0 0 180 0 1
device=terminal
T 54890 44200 5 10 0 0 180 0 1
footprint=CONNECTOR 1 1
T 54950 44750 5 10 1 1 180 6 1
refdes=A -
}
C 40600 48400 1 270 0 voltage-3.sym
{
T 41300 48200 5 8 0 0 270 0 1
device=VOLTAGE_SOURCE
T 41100 48100 5 10 1 1 270 0 1
refdes=12V
}
C 45500 45900 1 90 0 resistor-2.sym
{
T 45150 46300 5 10 0 0 90 0 1
device=RESISTOR
T 45200 46100 5 10 1 1 90 0 1
refdes=R1
T 45500 45900 5 10 1 1 0 0 1
value=41,66k
}
N 45400 47900 45400 48300 4
N 45400 46800 45400 47000 4
C 47100 44700 1 90 0 zener-2.sym
{
T 46600 45100 5 10 0 0 90 0 1
device=ZENER_DIODE
T 46800 45000 5 10 1 1 90 0 1
refdes=Z 2.5V
}
N 40800 44700 40800 47500 4
N 40800 48400 40800 49900 4
N 40800 49900 51300 49900 4
C 47100 48500 1 90 0 resistor-2.sym
{
T 46750 48900 5 10 0 0 90 0 1
device=RESISTOR
T 46800 48700 5 10 1 1 90 0 1
refdes=200k
}
N 47000 49900 47000 49400 4
N 47000 48500 47000 45500 4
N 45400 45900 47000 45900 4
N 45400 46900 49100 46900 4
C 49100 46300 1 0 0 opamp-1.sym
{
T 49800 47100 5 10 0 0 0 0 1
device=OPAMP
T 49800 46900 5 10 1 1 0 0 1
refdes=U (inf)
T 49800 47700 5 10 0 0 0 0 1
symversion=0.1
}
C 51400 48900 1 90 0 resistor-2.sym
{
T 51050 49300 5 10 0 0 90 0 1
device=RESISTOR
T 51100 49100 5 10 1 1 90 0 1
refdes=140k
}
C 51400 47500 1 90 0 resistor-2.sym
{
T 51050 47900 5 10 0 0 90 0 1
device=RESISTOR
T 51100 47700 5 10 1 1 90 0 1
refdes=100k
}
N 51300 49900 51300 49800 4
N 51300 48900 51300 48400 4
N 51300 47500 51300 44700 4
N 51300 48600 49600 48600 4
N 49600 48600 49600 47100 4
N 49600 46300 49600 44700 4
C 48800 45000 1 90 0 resistor-2.sym
{
T 48450 45400 5 10 0 0 90 0 1
device=RESISTOR
T 48500 45200 5 10 1 1 90 0 1
refdes=140k
}
C 50800 46000 1 180 0 resistor-2.sym
{
T 50400 45650 5 10 0 0 180 0 1
device=RESISTOR
T 50600 45700 5 10 1 1 180 0 1
refdes=100k
}
C 52900 46800 1 180 0 resistor-2.sym
{
T 52500 46450 5 10 0 0 180 0 1
device=RESISTOR
T 52700 46500 5 10 1 1 180 0 1
refdes=100k
}
N 50100 46700 52000 46700 4
N 50900 46700 50900 45900 4
N 50900 45900 50800 45900 4
N 49900 45900 48700 45900 4
N 48700 45000 48700 44700 4
N 48700 45900 48700 46500 4
N 48700 46500 49100 46500 4
N 52900 46700 54300 46700 4
T 49600 48700 9 10 1 0 0 0 1
+5V
T 46300 46000 9 10 1 0 0 0 1
+ 2.5 V
T 45700 48700 9 10 1 0 0 0 1
+/- 3.5 V