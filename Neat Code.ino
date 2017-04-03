#include <SoftwareSerial.h>

#define LS A1      // left sensor
#define RS A0      // right sensor

#define trigPin 8       //Reciever Triggers Shorted To Pin8
#define echoPin1 2       //Reciever1 Echo
#define echoPin2 3      //Reciever2 Echo

#define LMF 6
#define LMB 11

#define RMF 10
#define RMB 5

SoftwareSerial BTserial(12, 13); // RX | TX

volatile char control = 'X' ;

volatile long start = 0 ;
volatile long en = 0 ;

volatile int found = 0 ;
volatile int finished = 0 ;
volatile int finishedNew = 0 ;

volatile long startNew = 0 ;
volatile long endNew = 0 ;

volatile long diff1 = 0 ;
volatile long diff2 = 0 ;

volatile long duration = 0 ;
volatile long distance = 0 ;

char direxn = 'X' ;

void brake() ;
void forward() ;
void left() ;
void right() ;
void brake () ;
void getDirexn1 ( long timeDiff ) ;
void getDirexn2 ( long timeDiff ) ;
int avail() ;
void rst() ;


void setup() {
  Serial.begin(9600) ;

  BTserial.begin(115200);
  BTserial.print("$$$");
  delay(100);
  BTserial.println("U,9600,N");
  BTserial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(echoPin2, INPUT);

  pinMode ( 6 , OUTPUT ) ;
  pinMode ( 11 , OUTPUT ) ;
  pinMode ( 10 , OUTPUT ) ;
  pinMode ( 5 , OUTPUT ) ;

  pinMode ( LS , INPUT ) ;
  pinMode ( RS , INPUT ) ;

  attachInterrupt ( 0 , pinISR , CHANGE ) ;
  attachInterrupt ( 1 , pinISR2 , CHANGE ) ;

}

void loop() {
  if ( BTserial.available() > 0 || control == 'A' || control == 'M' ) {

    int data = BTserial.read() ;
    if ( data == 65 ) {
      Serial.println ("**ENTERING AUTO MODE**") ;
      control = 'A' ;
    } else if ( data == 66 ) {
      Serial.println ("**DISCONNECTING**") ;
      control = 'B' ;
    } else if ( data == 'M' ) {
      Serial.println ("**ENTERING MANUAL MODE**") ;
      control = 'M' ;
    }
    Serial.print("I RECIEVED : ") ;
    Serial.println(data) ;
    Serial.print ("CONTROL : ");
    Serial.println ( control ) ;

    if ( control == 'A' ) {
      digitalWrite(trigPin, HIGH);
      digitalWrite(trigPin, LOW);

      if ( avail() == 0 ) {
        brake() ;
        Serial.println ( "NO SIGNAL" );
        BTserial.println("V");
      }  else {
        if ( found == 1 ) {
          if ( finished == 1 ) {
            finishedNew = endNew - startNew ;
            getDirexn1 ( finishedNew ) ;
            rst () ;
          }
        } else if ( found == 2 ) {
          if ( finished == 1 ) {
            finishedNew = endNew - startNew ;
            getDirexn2 ( finishedNew ) ;
            rst () ;
          }
        }
      }
    } else if ( control == 'M' ) {
      if ( data == 'U' ) {
        Serial.println("MANUALLY FORWARD");
        forward() ;
      } else if ( data == 'D' ) {
        Serial.println("MANUALLY BACKWARD");
        back() ;
      } else if ( data == 'L' ) {
        Serial.println("MANUALLY LEFT");
        left();
      } else if ( data == 'R' ) {
        Serial.println("MANUALLY RIGHT");
        right() ;
      } else if ( data == 'S' ) {
        Serial.println("STOPPED");
        brake() ;
      }
    }
    delay(500);
  } else if ( control == 'B' ) {
    Serial.println("GOING BACK TO TROLLEY PARK");
    if ( digitalRead(LS) && digitalRead(RS) ){
      forward() ;
    } else if ( !digitalRead(LS) && digitalRead(RS) ){
      right() ;
    } else if ( digitalRead(LS) && !digitalRead(RS) ){
      left() ;
    } else if ( !digitalRead(LS) && !digitalRead(RS) ){
      brake() ;
      control = 'X' ;
    }
  } else {
    Serial.println ( "NO CONNEXN" ) ;
  }
}

void getDirexn1 ( long timeDiff ) {
  if ( timeDiff >= 0 && timeDiff <= 200 ) {
    forward() ;
    Serial.println ( "FORWARD" );
  } else {
    left() ;
    Serial.println ( "LEFT" );
  }
}

void getDirexn2 ( long timeDiff ) {
  if ( timeDiff >= 0 && timeDiff <= 200 ) {
    forward() ;
    Serial.println ( "FORWARD" );
  } else {
    right() ;
    Serial.println ( "RIGHT" );
  }
}

void pinISR() {
  if ( digitalRead ( echoPin1 ) ) {
    start = micros () ;
  }
  else {
    en = micros () ;
    diff1 = en - start ;
    if ( found == 0  ) {
      startNew = micros() ;
      found = 1 ;
    } else if ( found == 2 && !finished ) {
      endNew = micros () ;
      finished = 1 ;
    }
  }
}

void pinISR2() {
  if ( digitalRead ( echoPin2 ) ) {
    start = micros () ;
  }
  else {
    en = micros () ;
    diff2 = en - start ;
    if ( found == 0 ) {
      startNew = micros() ;
      found = 2 ;
    } else if ( found == 1 && !finished ) {
      endNew = micros () ;
      finished = 1 ;
    }
  }
}

void rst() {
  finished = 0 ;
  found = 0 ;
  endNew = 0 ;
  startNew = 0 ;
  finishedNew = 0 ;
}

int avail() {
  if ( diff1 >= 8500 && diff2 >= 8500 ) {
    digitalWrite ( 13 , !digitalRead ( 13 ) ) ;
    return 0 ;
  } else {
    digitalWrite ( 13 , LOW ) ;
    return 1 ;
  }
}

void brake() {
  direxn = 'X' ;

  digitalWrite ( 11 , LOW ) ;
  digitalWrite ( 6 , LOW ) ;

  digitalWrite ( 5 , LOW ) ;
  digitalWrite ( 10 , LOW ) ;
}

void forward() {
  if ( direxn != 'S' ){
    direxn = 'S' ;
    digitalWrite ( 11 , LOW ) ;
    digitalWrite ( 6 , HIGH ) ;

    digitalWrite ( 10 , LOW ) ;
    digitalWrite ( 5 , HIGH ) ;
  }
}

void right() {
  if ( direxn != 'R' ){
    direxn = 'R' ;
    digitalWrite ( 11 , LOW ) ;
    analogWrite ( 6 , 255 ) ;

    digitalWrite ( 10 , LOW ) ; 
    digitalWrite ( 5 , LOW ) ;
  }
}

void left() {
  if ( direxn != 'L' ){
    direxn = 'L' ;
    analogWrite ( 11 , LOW  ) ; 
    digitalWrite ( 6 , LOW ) ;

    digitalWrite ( 10 , LOW ) ;
    analogWrite ( 5 , 230 ) ;
  }
}

void back() {
  if ( direxn != 'B' ){
    direxn = 'B' ;
    analogWrite ( 11 , HIGH  ) ; 
    digitalWrite ( 6 , LOW ) ;

    digitalWrite ( 10 , HIGH ) ;
    digitalWrite ( 5 , LOW ) ;
  }
}

