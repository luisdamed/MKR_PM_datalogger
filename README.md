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
Date     Time	   PM2.5	PM10	Temp	RelHum	Press
01/01/21 22:56:05	8.70	11.20	25.80	37.18	75.73	
01/01/21 22:56:06	8.90	11.50	24.28	38.05	98.08	
01/01/21 22:56:08	9.00	11.60	24.32	38.12	98.08	
01/01/21 22:56:09	9.20	12.00	24.32	38.28	98.08	
01/01/21 22:56:11	9.10	11.90	24.24	38.34	98.08	
01/01/21 22:57:28	9.50	12.40	24.22	38.39	98.08	
01/01/21 22:57:30	9.40	12.20	23.24	38.77	98.08	
01/01/21 22:57:31	9.30	12.00	23.18	38.74	98.08	
01/01/21 22:57:33	9.00	12.00	23.18	38.82	98.08	
01/01/21 22:57:34	9.20	12.20	23.24	38.74	98.08	
01/01/21 22:58:51	8.70	13.00	23.22	38.84	98.08	
01/01/21 22:58:53	8.80	13.10	22.59	39.82	98.08	
01/01/21 22:58:54	9.00	13.30	22.61	39.87	98.08	
01/01/21 22:58:56	9.00	13.10	22.61	39.94	98.08	
01/01/21 22:58:57	9.00	13.50	22.57	39.93	98.08
01/01/21 23:00:14	9.40	13.10	22.52	39.92	98.08	
01/01/21 23:00:16	9.70	13.70	22.16	40.54	98.08	
01/01/21 23:00:17	9.60	13.30	22.14	40.45	98.08	
01/01/21 23:00:19	9.60	13.10	22.18	40.50	98.08	
01/01/21 23:00:20	9.60	13.20	22.16	40.52	98.08	
```

***



## Assembly

The datalogger can be assembled by the course students without soldering nor special tools. Only jumper wires are required. 

An optional (yet, convenient) 3D printed frame can be used to accomodate all the electronics. The file
  
