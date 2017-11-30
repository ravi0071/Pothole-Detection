# Pothole-Detection
ESP32 codes

cal_dist: this code calculates distance between two locations using latitude and longitude.

gps: this code displays the location on serial monitor.

mpu6050: this code displays the acceleration values on serial monitor in meter per seconds.

send_value: This code uploads the gps location to Thingspeak.

recive_test: This code gets the gps data from Thingspeak and displays on serial monitor.

final: This code is combination of all, it uploads new pothole location to thingspeak and recives pothole location already stored in thingspeak and alerts user about upcomming potholes.
