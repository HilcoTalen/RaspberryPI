#RaspberryPI

Application which reads the Hewalex Sunboiler, and the Intergas heater; readible via ModBUS.

The Hewalex Sunboiler is connected via RS232, via an FTDI <> RS485 Converter The Intergas module is connected via an FTDI <> RS232 module. The ModBUS is connected via an FTD <> RS485 Converter.

More info will come; also the code will be improved, as this is just the first scratch.

The application is written in C++ in a Visual Studio 2022 IDE. This is cross-compiled on a Raspberry PI (3B), with the default Raspbian image.

# ModBUS address table

#Addres 1 (The server)

| Address | Name                 |
| ------- | -------------------- |
| 1       | Intergas Sent        |
| 2       | Intergas Received    |
| 3       | Intergas Timeouts    |
| 4       | Hewalex Sent         |
| 5       | Hewalex Received     |
| 6       | Hewalex Timeouts     |
| 7       | P1 Sent (not used)   |
| 8       | P1 Received          |
| 9       | P1 Timeout           |
| 10      | Status               |

#Address 2 (Intergas)

| Address | Name                           |
| ------- | ------------------------------ |
| 2       | Exhaust temperature            |
| 4       | Source temperature             |
| 6       | Return temperature             |
| 8       | Hot water temperature          |
| 10      | Boiler temperature             |
| 12      | Outside temperature            |
| 14      | Pressure                       |
| 16      | Temperature setpoint           |
| 18      | Fan speed setpoint             |
| 20      | Fan speed                      |
| 22      | Fan PWM                        |
| 24      | Ionisation current             |
| 26      | Status code                    |
| 28      | Status byte 1                  |
| 30      | Status byte 2                  |
| 36      | Line power connected           |
| 38      | Power connected times          |
| 40      | Heating hours                  |
| 42      | Hot water hours                |
| 44      | Burner starts (total)          |
| 46      | Ignition failed                |
| 48      | Flame loss                     |
| 50      | Resets                         |
| 52      | Gas meter heating              |
| 54      | Gas meter hot water            |
| 56      | Water meter                    |
| 58      | Burner starts heating (Hot water) |
| 60      | Heater on                      |
| 61      | Comfort mode                   |
| 62      | Central heating setpoint       |
| 63      | Hot water setpoint             |
| 64      | Eco days                       |
| 65      | Comfort setpoint               |
| 66      | Hot water night                |
| 67      | Central heating night          |
| 68      | Parameter 1                    |
| 69      | Parameter 2                    |
| 70      | Parameter 3                    |
| 71      | Parameter 4                    |
| 72      | Parameter 5                    |
| 73      | Parameter 6                    |
| 74      | Parameter 7                    |
| 75      | Parameter 8                    |
| 76      | Parameter 9                    |
| 77      | Parameter A                    |
| 78      | Parameter B                    |
| 79      | Parameter C                    |
| 80      | Parameter C                    |
| 81      | Parameter D                    |
| 82      | Parameter E                    |
| 83      | Parameter E.                   |
| 84      | Parameter F                    |
| 85      | Parameter H                    |
| 86      | Parameter N                    |
| 87      | Parameter O                    |
| 88      | Parameter P                    |
| 90      | Parameter F.                   |
| 91      | Parameter heating flow         |
| 92      | Parameter O.                   |
| 93      | Cascade reaction               |
| 94      | Parameter 5.                   |
| 95      | Parameter C.                   |
| 96      | Parameter 3.                   |
| 97      | Parameter P.                   |
| 100     | Fault 1 times                  |
| 101     | Fault 1 times                  |
| 102     | Fault 2 times                  |
| 103     | Fault 3 times                  |
| 104     | Fault 4 times                  |
| 105     | Fault 5 times                  |
| 106     | Fault 6 times                  |
| 107     | Fault 7 times                  |
| 108     | Fault 8 times                  |
| 109     | Fault 9 times                  |
| 110     | Fault 10 times                 |
| 111     | Fault 11 times                 |
| 112     | Fault 12 times                 |
| 113     | Fault 13 times                 |
| 114     | Fault 14 times                 |
| 115     | Fault 15 times                 |
| 116     | Fault 16 times                 |
| 117     | Fault 17 times                 |
| 118     | Fault 18 times                 |
| 119     | Fault 19 times                 |
| 120     | Fault 20 times                 |
| 121     | Fault 21 times                 |
| 122     | Fault 22 times                 |
| 123     | Fault 23 times                 |
| 124     | Fault 24 times                 |
| 125     | Fault 25 times                 |
| 126     | Fault 26 times                 |
| 127     | Fault 27 times                 |
| 128     | Fault 28 times                 |
| 129     | Fault 29 times                 |
| 130     | Fault 30 times                 |
| 131     | Fault 31 times                 |

#Addres 3 (Hewalex Sunboiler)
| Address | Name                            |
| ------- | ------------------------------- |
| 120     | Date                            |
| 124     | Time                            |
| 128     | T1                              |
| 130     | T2                              |
| 132     | T3                              |
| 134     | T4                              |
| 136     | T5                              |
| 138     | T6                              |
| 144     | Collector Power (W)             |
| 148     | Consumption                     |
| 150     | Collector active                |
| 152     | Flow Rate                       |
| 154     | Pumps                           |
| 156     | Collector Pump Speed            |
| 166     | Total Energy                    |
| 170     | Installation Scheme             |
| 172     | Display Timeout                 |
| 174     | Display Brightness              |
| 176     | Alarm Sound Enabled             |
| 178     | Key Sound Enabled               |
| 180     | Display Language                |
| 182     | Fluid Freezing Temp              |
| 186     | Flow Rate Nominal               |
| 188     | Flow Rate Measurement            |
| 190     | Flow Rate Weight                |
| 192     | Holiday Enabled                 |
| 194     | Holiday Start Day               |
| 196     | Holiday Start Month             |
| 198     | Holiday Start Year              |
| 200     | Holiday End Day                 |
| 202     | Holiday End Month               |
| 204     | Holiday End Year                |
| 206     | Collector Type                  |
| 208     | Collector Pump Hysteresis       |
| 210     | Extra Pump Hysteresis           |
| 212     | Collector Pump Max Temp         |
| 214     | Boiler Pump Min Temp            |
| 218     | Heat Source Max Temp            |
| 220     | Boiler Pump Max Temp            |
| 222     | Pump Regulation Enabled         |
| 226     | Heat Source Max Collector Power |
| 228     | Collector Overheat Prot Enabled |
| 230     | Collector Overheat Prot Max Temp |
| 232     | Collector Freezing Prot Enabled |
| 234     | Heating Priority                |
| 236     | Legionella Prot Enabled         |
| 238     | Lock Boiler K With Boiler C     |
| 240     | Night Cooling Enabled           |
| 242     | Night Cooling Start Temp        |
| 244     | Night Cooling Stop Temp         |
| 246     | Night Cooling Stop Time         |
| 248     | Time Program CM-F               |
| 252     | Time Program CSat               |
| 256     | Time Program CSun               |
| 260     | Time Program KM-F               |
| 264     | Time Program KSat               |
| 268     | Time Program KSun               |
| 278     | Collector Pump Min Rev          |
| 280     | Collector Pump Max Rev          |
| 282     | Collector Pump Min Inc Time     |
| 284     | Collector Pump Min Dec Time     |
| 286     | Collector Pump Startup Speed    |
| 288     | Pressure Switch Enabled         |
| 290     | Tank Overheat Prot Enabled      |
| 322     | Circulation Pump Enabled        |
| 324     | Circulation Pump Mode           |
| 326     | Circulation Pump Min Temp       |
| 328     | Circulation Pump ON Time        |
| 330     | Circulation Pump OFF Time       |
| 312     | Total Operation Time            |
| 320     | Reg320                          |

# Address 4 (P1 Meter)
| Address | Name                          |
| ------- | ----------------------------- |
| 2       | Consumed low tariff           |
| 4       | Consumed high tariff          |
| 6       | Returned low tariff           |
| 8       | Returned high tariff          |
| 10      | Short power failure count     |
| 12      | Long power failure count      |
| 14      | Short power failure duration  |
| 16      | Long power failure duration   |
| 18      | Gas                           |
| 20      | Actual consumption            |
| 22      | Actual return                 |
| 24      | Actual consumption L1         |
| 26      | Actual consumption L2         |
| 28      | Actual consumption L3         |
| 30      | Actual return L1              |
| 32      | Actual return L2              |
| 34      | Actual return L3              |
| 36      | Current L1                    |
| 38      | Current L2                    |
| 40      | Current L3                    |
| 42      | Voltage L1                    |
| 44      | Voltage L2                    |
| 46      | Voltage L3                    |
| 48      | Tariff                        |

