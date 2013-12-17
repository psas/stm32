#!/usr/bin/python
from datetime import *
import math
todaysDate = date.today()
currentTime = datetime.now()
#get the date in binary
#this is ugly and could possibly be written better
#get the date part of the bit vector for rtc_dr (bits 0-7, 6 and 7 are reserved)
dateBitsOnes = 0
dateBitsTens = 0
if todaysDate.day >= 30:
	dateBitsOnes =  todaysDate.day - 30
	dateBitsTens = int(math.floor(todaysDate.day/10))
elif 30 > todaysDate.day >= 20:
	dateBitsOnes =  todaysDate.day - 20
	dateBitsTens = int(math.floor(todaysDate.day/10))
elif 20 > todaysDate.day >= 10:
	dateBitsOnes = todaysDate.day - 10
	dateBitsTens = int(math.floor(todaysDate.day/10))
else:
	dateBitsOnes = todaysDate.day
	dateBitsTens = 0
dateBitsTens = dateBitsTens << 3
binDate = dateBitsTens | dateBitsOnes & 0xFF #date of the month in binary 
monthBitsOnes = 0
monthBitsTens = 0
if todaysDate.month >= 12:
	monthBitsOnes = todaysDate.month - 10
	monthBitsTens = int(math.floor(todaysDate.month/10))
else:
	monthBitsOnes = todaysDate.month
	monthBitsTens = 0
monthBitsTens = monthBitsTens << 4
binMonth = monthBitsTens | monthBitsOnes & 0xFF
dayOfWeek = (todaysDate.weekday() + 1) & 0x7
#get the year in binary
#good for the next few years
yearBitsOnes = todaysDate.year - 2010
yearBitsTens = int(math.floor((todaysDate.year - 2000 )/10))
yearBitsTens = yearBitsTens << 4
binYear = yearBitsTens | yearBitsOnes & 0x1F
#now lets put it together into one big vector
binMonth = binMonth << 8
rtc_dr = binMonth | binDate
dayOfWeek = dayOfWeek << 12
rtc_dr = dayOfWeek | rtc_dr
binYear = binYear << 16
rtc_dr = (binYear | rtc_dr) & 0xFFFFFFFF

print bin(rtc_dr)

#get the time in binary
#seconds
secBitsOnes = 0
secBitsTens = int(math.floor(currentTime.second)/10)
if currentTime.second >= 50:
	secBitsOnes = currentTime.second - 50
elif 50 > currentTime.second >= 40:
	secBitsOnes = currentTime.second - 40
elif 40 > currentTime.second >= 30:
	secBitsOnes = currentTime.second - 30
elif 30 > currentTime.second >= 20:
	secBitsOnes = currentTime.second - 20
elif 20 > currentTime.second >= 20:
	secBitsOnes - currentTime.second - 10
else:
	secBitsOnes = currentTime.second

secBitsTens = secBitsTens << 4
binSecs = secBitsTens | secBitsOnes & 0xFF
#minutes
minBitsOnes = 0
minBitsTens = int(math.floor(currentTime.minute/10))
if currentTime.minute >= 50:
	minBitsOnes = currentTime.minute - 50
elif 50 >  currentTime.minute >= 40:
	minBitsOnes = currentTime.minute - 40
elif 40 > currentTime.minute >= 30:
	minBitsOnes = currentTime.minute - 30
elif 30 > currentTime.minute >= 20:
	minBitsOnes = currentTime.minute - 20
elif 20 > currentTime.minute >= 10:
	minBitsOnes = currentTime.minute - 20
else:
	minBitsOnes = currentTime.minute
minBitsTens = minBitsTens << 4
binMins = minBitsTens | minBitsOnes & 0xFF
#hours
hourBitsOnes = 0
hourBitsTens = int(math.floor(currentTime.hour/10))
if currentTime.hour >= 20:
	hourBitsOnes = currentTime.hour - 20
elif 20 > currentTime.hour >= 10:
	hourBitsOnes = currentTime.hour - 10
else:
	hourBitsOnes = currentTime.hour
hourBitsTens = hourBitsTens << 4
binHours = hourBitsTens | hourBitsOnes & 0x3F

binMins = binMins << 8
rtc_dr = binMins | binSecs
binHours = binHours << 16
rtc_dr = (binHours | rtc_dr) & 0xFFFFFFFF
print bin(rtc_dr)

