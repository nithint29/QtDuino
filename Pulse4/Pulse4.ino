#define countPin  2
#define r1Pin  4
#define r2Pin  5
#define r3Pin  6
#define SIGNPIN  7

#define pulseWidth  1000
#define counterMode 'C'
#define stepperMode 'S'
#define pulseSender1  8
#define pulseSender2  9
#define pulseSender3  10
#define pulseSender4  11
#define bPin 19
#define onPin 3

int box[3] = {0,0,0};
int pulsePin = 4;
//int pulseSender = 8;
int ledPin = LED_BUILTIN;
int in;
char mode;
volatile int count = 0;

//counter stuff
int points;
int stepTime;  //in milli seconds
int stepsPerPoint;

int motorPin1 = 8;
int motorPin2 = 9;
int motorPin3 = 10;
int motorPin4 = 11;
int delayTime = 20;
int b = 10000;                      //number of steps
//int count = 0;
//int upordown =1;               //to keep track which way we were moving in the previus command //1 is up, 2 is down - save in the text file
//int i=1;                        //this is a coil number needed to keep track which coil to active next so we do not loose any steps - save in the text file
int prevCoil = 0;   //previously pulsed coil, will need to be stored in text file
int currentCount = 0; //will need to be stored in text file maybe


void setup() 
{
  attachInterrupt(digitalPinToInterrupt(countPin),counter,FALLING);
  pinMode(digitalPinToInterrupt(countPin), INPUT_PULLUP);


  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
  
  Serial.begin(9600);
  //Serial.println("Arduino is ready!");
}

void loop() 
{
  if(Serial.available()>0)
  {
    //String input = Serial.readString();
    mode = Serial.read();
    //currentCount = Serial.read();
    

    if(mode == stepperMode)
    {
      prevCoil = Serial.parseInt();
      
      pulsePin = Serial.parseInt(); //S: pulse pin
      
      //digitalWrite(pulsePin,HIGH); // choose the right box
      in = Serial.parseInt();//S:number of pulses to send out
      
      if(in != 0 && pulsePin <= 6 && pulsePin >= 4)
      {
        Serial.print("I received: ");
        Serial.print(pulsePin);//Serial.print(c);
        Serial.println(in);
        //pulsePin = LED_BUILTIN;  // REMOVE WHEN USING
        pulse(in);
        mode = 'N';
      }
    }

    else if(mode == counterMode)
    {
      pulsePin = Serial.parseInt();
      
      stepsPerPoint = Serial.parseInt();
      stepTime = Serial.parseInt();
      points = Serial.parseInt();
      Serial.print("I am a counter: ");Serial.print(stepsPerPoint);Serial.print(",");Serial.print(stepTime);
      Serial.print(",");Serial.print(points);Serial.print(",Pin:");Serial.println(pulsePin);

      for(int i = 0; i<points;i++)
      {
        count = 0;
        pulse(stepsPerPoint);
        delay(stepTime);
        Serial.println(count);
      }
      mode = 'N';
      
    }
  }

}
int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}
void pulse(int a) //send pulses to pulseSender
{
  int temp = count;
   //Serial.println(a);
  //digitalWrite(pulsePin,HIGH);
  if(a<0)
  {
    //digitalWrite(SIGNPIN,LOW);
    for(long i = 0;i>(a);i--)
    {
      int index = 8+(mod((i+prevCoil-1),4));
      digitalWrite(index,HIGH);
      //Serial.println(8+(i%4));
      //Serial.print("count: ");Serial.println(i);
      delay(pulseWidth);
      digitalWrite(index,LOW);
    }
    prevCoil = mod(-1*(prevCoil+a),4);
    
  }
  else
  {
    //digitalWrite(SIGNPIN,HIGH);
    for(long i = 0;i<a;i++)
    {
      int index = 8+(mod((i+1+prevCoil),4));
      digitalWrite(index,HIGH);
      //Serial.println(8+(i%4));
      //Serial.print("count: ");Serial.println(i);
      delay(pulseWidth);
      digitalWrite(index,LOW);
    }
      prevCoil = mod((prevCoil+a),4);
  }

  count = temp;

}

void counter()
{
  count++;
}

