#define D1_TRIG 4                     // Distance Sensor 1 Trig Pin
#define D1_ECHO 5                     // Distance Sensor 1 Echo Pin
#define D2_TRIG 6                     // Distance Sensor 2 Trig Pin
#define D2_ECHO 7                     // Distance Sensor 2 Echo Pin
#define SPEED_OF_SOUND 0.034          // Speed of sound (cm/uS)

float baseline = 100;                 // Distance between the transducers (cm)
float distance1 = 0.0;                // From active sender (cm)
float distance2 = 0.0;                // From passive receiver (cm)
int task_timer = 0;                   // Task timer
bool task_flag = false;               // Task flag

void setup(){
  Serial.begin(9600);

  // Set pins
  pinMode(D1_ECHO, INPUT);
  pinMode(D2_ECHO, INPUT);
  pinMode(D1_TRIG, OUTPUT);
  pinMode(D2_TRIG, OUTPUT);

  digitalWrite(D1_TRIG, LOW);
  digitalWrite(D2_TRIG, LOW);

  // Configure timer to generate a compare-match interrupt every 1mS
  noInterrupts();                     // Disable interrupts
  TCCR2A = 0;                         // Clear control registers
  TCCR2B = 0;
  TCCR2B |= (1 << CS22) |             // 16MHz/128=8uS
            (1 << CS20) ;
  TCNT2 = 0;                          // Clear counter
  OCR2A = 125 - 1;                    // 8uS*125=1mS (allow for clock propagation)
  TIMSK2 |= (1 << OCIE2A);            // Enable output compare interrupt
  interrupts();                       // Enable interrupts
}

void loop(){
  // Measure object distances
  if(task_flag){
    task_flag = false;
    measure();

    // Send distance1 and distance2 to serial
    Serial.print(distance1); Serial.print(","); Serial.println(distance2);
  }
}

// Task scheduler (1mS interrupt)
ISR(TIMER2_COMPA_vect){
  task_timer++;

  if(task_timer > 499){               // Interval between pings (50mS=423cm)
    task_timer = 0;                   // Reset timer
    task_flag = true;                 // Signal loop() to perform task
  }
}

void measure(){
  // Variables to track time (ms) and echo line states
  unsigned long time_start;
  unsigned long time1_finish;
  unsigned long time2_finish;
  unsigned long time_delta;
  boolean echo1_recieved_flag;
  boolean echo2_recieved_flag;

  // Send 10uS trigger pulse
  digitalWrite(D1_TRIG, HIGH);
  digitalWrite(D2_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(D1_TRIG, LOW);
  digitalWrite(D2_TRIG, LOW);

  // Wait for both echo lines to go high
  while(!digitalRead(D1_ECHO));
  while(!digitalRead(D2_ECHO));

  // Save start time
  time_start = micros();

  // Reset the flags
  echo1_recieved_flag = false;
  echo2_recieved_flag = false;

  // Measure echo times
  while((!echo1_recieved_flag) || (!echo2_recieved_flag)){
    
    // If Echo 1 is recieved
    if((!echo1_recieved_flag) && (!digitalRead(D1_ECHO))){
      echo1_recieved_flag = true;
      time1_finish = micros();
      time_delta = time1_finish - time_start;

      // Distance = Time * Velocity 
      // Multiply by 2 to include the return distance
      distance1 = ((float) time_delta) * (SPEED_OF_SOUND * 2);
    }

    // If Echo 2 is recieved
    if((!echo2_recieved_flag) && (!digitalRead(D2_ECHO))){
      echo2_recieved_flag = true;
      time2_finish = micros();
      time_delta = time2_finish - time_start;

      // Distance = Time * Velocity
      distance2 = ((float) time_delta) * SPEED_OF_SOUND;
    }
  }
}
