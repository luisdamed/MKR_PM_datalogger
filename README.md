# MKR_PM_datalogger
 
 This datalogger module has been developed for use in the laboratory project of the course [Aerosol technology and air quality (ATAQ)](https://didattica.polito.it/pls/portal30/gap.pkg_guide.viewGap?p_cod_ins=01USDND&p_a_acc=2021&p_header=S&p_lang=IT "ATAQ course description at PoliTo's website") from Politecnico di Torino (01USDND, 01USDND, 01USDNE) in the academic year 2020-2021.

 This datalogger module is based on Arduino MKR WiFi 1010, MKR ENV Shield and SDS011 PM sensor.
 Currently, the WiFi functionality is only used to synchronize RTCZero. This will be expanded in the future.






[Aerosol Technology Research Group](http://aerosoltech.polito.it/ "Aerosol Technology Research Group")

***



## Output


The readings from the sensors are logged in tab-separated text file, stored in the microSD card.
This can be imported as a table into Excel or Matlab, for example, to analyze it.
Sample output with basic usage:

```
Date     Time   	PM2.5	PM10	Temp	RelHum	Press
15/05/21 11:56:35	6.50	9.80	24.23	50.58	98.36	
15/05/21 11:56:37	6.40	9.90	24.45	50.73	98.36	
15/05/21 11:56:39	6.40	9.50	24.47	50.77	98.36	
15/05/21 11:56:40	6.50	9.50	24.45	50.71	98.36	
15/05/21 11:56:42	6.50	9.50	24.49	50.76	98.36	
15/05/21 11:56:42	6.50	9.50	24.49	50.76	98.36	

```

***



## Assembly

The datalogger can be assembled by the course students without soldering nor special tools. Only jumper wires are required. 

A 3D printed frame can be used to accomodate all the electronics.
  
