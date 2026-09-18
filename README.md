# 2026APLANT
Robot and Controller Code for "Roomba of Death" competing at  WPI's 2026 A-Term Plastic Antweight combat competition

## How it works
Robot has esp32 set up as a WiFi acces point and communicates with the controller using UDP, the controller pings the robot to keep the connection alive, if the robot does not recieve any pings for a certain amount of time it get recognized as a disconect and stops everyhing, as specified in the rules.
