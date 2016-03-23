#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
char inChar; 
int motor_delay = 50;
float tolerance = 5.0;
float desired_heading = 70.0 ;

#include <IRremote.h>

int RECV_PIN = 2 ;

IRrecv irrecv(RECV_PIN);

decode_results results;


void maintainheading(float desired,float tolerance,float present)
{
      if(abs(desired - present)<= tolerance)
      {
          drive_forward();
      }
      else if(present<desired-tolerance)
      {
          full_right();
          
      }
      else if(present>desired+tolerance)
      {
          full_left();
      }
      delay(motor_delay);
      motor_stop();
}

/* Assign a unique ID to this sensor at the same time */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
int motor_left[] = {11, 12};
int motor_right[] = {10, 9};

void motor_stop(){
digitalWrite(motor_left[0], LOW); 
digitalWrite(motor_left[1], LOW); 
//Serial.println("Stop");
digitalWrite(motor_right[0], LOW); 
digitalWrite(motor_right[1], LOW);
delay(25);
}

void drive_forward(){
digitalWrite(motor_left[0], HIGH); 
digitalWrite(motor_left[1], LOW); 
//Serial.println("Forward");
digitalWrite(motor_right[0], HIGH); 
digitalWrite(motor_right[1], LOW); 
}

void drive_backward(){
digitalWrite(motor_left[0], LOW); 
digitalWrite(motor_left[1], HIGH); 
//Serial.println("Backward");
digitalWrite(motor_right[0], LOW); 
digitalWrite(motor_right[1], HIGH); 
}

void turn_left(){
digitalWrite(motor_left[0], LOW); 
digitalWrite(motor_left[1], LOW); 
//Serial.println("Left");
digitalWrite(motor_right[0], HIGH); 
digitalWrite(motor_right[1], LOW);
}

void turn_right(){
digitalWrite(motor_left[0], HIGH); 
digitalWrite(motor_left[1], LOW); 
//Serial.println("Right");
digitalWrite(motor_right[0], LOW); 
digitalWrite(motor_right[1], LOW); 
}

void full_left(){
digitalWrite(motor_left[0], LOW); 
digitalWrite(motor_left[1], HIGH); 
//Serial.println("Full Left");
digitalWrite(motor_right[0], HIGH); 
digitalWrite(motor_right[1], LOW);
}

void full_right(){
digitalWrite(motor_left[0], HIGH); 
digitalWrite(motor_left[1], LOW); 
//Serial.println("Full Right");
digitalWrite(motor_right[0], LOW); 
digitalWrite(motor_right[1], HIGH); 
}
void displaySensorDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
  //Serial.println("------------------------------------");
  //Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  //Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  //Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  //Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  //Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  //Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  //Serial.println("------------------------------------");
  //Serial.println("");
  delay(500);
}

int decode_hex(String input)
{
 int zeros = 8-input.length();
 for(int i=0; i<zeros;i++)
 {
   input = "0" + input;    
 }
 String hex_data = input.substring(0,2) + input.substring(4,6);  
 //Serial.print("Real Hex data: ");
 //Serial.println(hex_data); 
 const char * c = hex_data.c_str();
 int output = (int) strtol(c, NULL, 16);
 return output;
}

String encode_hex(int input)
{
 String real_hex = String(input,HEX);
 int zeros = 4-real_hex.length();
 for(int i=0; i<zeros;i++)
 {
   real_hex = "0" + real_hex;    
 }
 
 char HEXADECIMAL[17] = "0123456789abcdef";
 
 String final_hex = real_hex.substring(0,2);
 for(int i = 0; i<2; i++)
 {
   for(int j = 0; j<16; j++)
   {
     if(real_hex.substring(0+i,1+i) == String(HEXADECIMAL[j]))
     {
       final_hex += String(HEXADECIMAL[15-j]);
     }
   }
 }
 
 final_hex += real_hex.substring(2);
 
 for(int i = 0; i<2; i++)
 {
   for(int j = 0; j<16; j++)
   {
     if(real_hex.substring(2+i,3+i) == String(HEXADECIMAL[j]))
     {
       final_hex += String(HEXADECIMAL[15-j]);
     }
   }
 }
 return final_hex;
  
}

void setup(void) 
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  //Serial.println("HMC5883 Magnetometer Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    //Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
  int i;
  for(i = 0; i < 2; i++)
  {
     pinMode(motor_left[i], OUTPUT);
     pinMode(motor_right[i], OUTPUT);
  }

  /* Display some basic information on this sensor */
  displaySensorDetails();
}

void loop(void) 
{
  if (irrecv.decode(&results)) {
   //Serial.println(results.value, HEX);
   String received = String(results.value,HEX);
   int angle = decode_hex(received);
   if(angle < 361 && angle > -1)desired_heading = angle;
   Serial.print("Received angle: ");
   Serial.println(angle);
   //Serial.println(encode_hex(angle));
   irrecv.resume(); // Receive the next value
 }

 
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
 
  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  //Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
  //Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
  //Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.println("uT");

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.22;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 
  
  //Serial.print("Present heading : "); Serial.print(headingDegrees); Serial.print("         -------------            Desired heading");Serial.println(desired_heading);
  maintainheading(desired_heading, tolerance, headingDegrees);
  delay(50);
}
