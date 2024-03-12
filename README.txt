============================================================================================
รายชื่อผู้พัฒนา

	1.) นายจารุกิตติ์ ผลวัฒนานุวงศ์
	2.) นายภีมพัฒน์ ทองมี
	3.) นายอธิภู พิมพิศาล
	4.) นายอลงกต วิมลสุข

ชื่อวิชา 01204114 การพัฒนาฮาร์ดแวร์คอมพิวเตอร์เบื้องต้น (Introduction to Computer Hardware Development)
ภาควิชา วิศวกรรมคอมพิวเตอร์
คณะวิศวกรรมศาสตร์ มหาวิทยาลัยเกษตรศาสตร์
============================================================================================

ไดเรคตอรี

1. โฟลเดอร์ source code แบ่งเป็น 5 โฟลเดอร์ย่อยคือ

	- Gunboard            	# เก็บไฟล์โค้ดสำหรับ ESP32-S3 ที่อยู่ในปืน
	- Targetboard1    	# เก็บไฟล์โค้ดสำหรับ ESP32-S3 ที่อยู่ในเป้าที่หนึ่ง
	- Targetboard2    	# เก็บไฟล์โค้ดสำหรับ ESP32-S3 ที่อยู่ในเป้าที่สอง
	- Targetboard3     	# เก็บไฟล์โค้ดสำหรับ ESP32-S3 ที่อยู่ในเป้าที่สาม
	- node_red_flow	 	# เก็บไฟล์ flow บน node-red

2. โฟลเดอร์ schematic

	- Schematic_hardwaredev	# เป็นไฟล์ที่เป็น schematic ของ ปืนและเป้าทั้งสาม

3. LICENSE.txt

4. README.txt

รายการไลบรารี่ที่ใช้

	- Adafruit BusIO by Adafruit
	- Adafruit GFX Library by Adafruit
	- Adafruit SSD1306 by Adafruit
	- IRremote by shirriff,z3t0,ArminJo
	- IRremoteESP8266 by David Conran, Sebastien Warin, Mark Szabo, Ken Shirriff
	- PubSubCLient by Nick O'Leary<nick.oleary@gmail.com>

รายการอุปกรณ์ฮาร์ดแวร์ที่ใช้

	- ESP32-S3 x 4
	- IR Transmitter x 1
	- IR Receiver x 3
	- buzzer x 2
	- Yellow LED x3
	- Green LED x 3
	- OLED x 1
	- button switch x 3
	- 18650 battery shield x4
