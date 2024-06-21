int distance1 = random(81);           // From active sender (cm)
int distance2 = random(81);           // From passive receiver (cm)
int task_timer = 0;                   // Task timer
bool task_flag = false;               // Task flag

void setup() {
  Serial.begin(9600);

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

void loop() {
  // Fake object distances
  if(task_flag){
    task_flag = false;
    fake_measure();

    // Send distance1 and distance2 to serial
    Serial.print(distance1); Serial.print(","); Serial.println(distance2);
  }
}

// Task scheduler (1mS interrupt)
ISR(TIMER2_COMPA_vect){
  task_timer++;

  // Task 1
  if(task_timer > 499){               // Interval between pings (50mS=423cm)
    task_timer = 0;                   // Reset timer
    task_flag = true;                 // Signal loop() to perform task
  }
}

void fake_measure(){
  // Randomly increase or decrease distances by 1
  int rand_x = random(-1, 2);
  int rand_y = random(-1, 2);

  distance1 += rand_x;
  distance2 += rand_y;
}