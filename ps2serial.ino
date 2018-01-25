/* PS2SERIAL - PS/2 to Serial mouse converter firmware
 * 2018 Meido-Tek Productions (Lameguy64)
 * Version 1.1
 * 
 * This sketch is for a PS/2 to Serial mouse converter circuit that would allow the use of
 * a modern PS/2 laser mouse on a vintage PC that does not support PS/2. Essentially a very
 * reliable and cost effective alternative to getting an original serial mouse.
 * 
 * This sketch requires the PS2Mouse library by Kristopher on Github:
 * https://github.com/kristopher/PS2-Mouse-Arduino
 * 
 * The schematic for the converter should be included along with this file.
 * 
 * Usage Instructions:
 *  - Upload this sketch to your converter's Arduino.
 *  - If PS/2 mouse is wired correctly, the L13 light should flash 4 times on start-up.
 *  - After the blink sequence, the converter should be detected by your vintage PC as a
 *    3-button Logitech mouse.
 *  - L13 light will blink every time the mouse driver tries to initialize the mouse (by
 *    toggling RTS) to indicate that the converter is working properly and the driver\
 *    should recognize it as a 3-button Logitech mouse.
 *    
 * Fixes in 1.1:
 *  - Fixed mouse button handling issues that made certain mouse operations (such as
 *    double-click) not work correctly.
 *  
 * Known bugs/issues so far:
 *  - There might be some slight input lag with the mouse movement. Might be because the
 *    Arduino is not fast enough (tested on a Leonardo with a ATmega 32u4).
 *  - Mouse input becomes slightly choppy while pressing down middle button due to
 *    the added delay of sending 4 byte packets for the 3rd button status instead of 3
 *    byte packets.
 *    
 *  As always, feel free to improve upon this chode and issue a pull request to merge with
 *  the main repository.
 */


#include <PS2Mouse.h>


/* Pin settings, feel free to change to what you find most convenient to use */

#define PS2_MOUSE_CLOCK   2   /* Must connect to Pin 5 (clock) of PS/2 mouse */
#define PS2_MOUSE_DATA    4   /* Must connect to Pin 1 (data) of PS/2 mouse */

#define RTS_PROBE         7   /* Must connect to Pin 7 (RTS) of PC serial port */
                              /* (NOTE: must go through a 5v regulator and */
                              /* 200ohm resistor first or you'll blow the pin) */

/* Serial data will always be transmitted in Pin 2 (Tx) of the Arduino but you must
 * connect it through a 74LS00 to invert the logic levels so the PC serial can
 * understand it. No MAX-232 required as 5v is enough to drive most serial interfaces.
 */

/* Uncomment to enable debug messages over USB serial (serial monitor must be active) */
//#define DEBUG


/* Status variables for keeping track of button/probe states */
bool left_status = false;
bool right_status = false;
bool middle_status = false;
bool rts_status = false;

int x_status = 0;
int y_status = 0;

/* Class from PS2Mouse for reading PS/2 mouse data */
PS2Mouse mouse( PS2_MOUSE_CLOCK, PS2_MOUSE_DATA, STREAM );


/* Serial mouse packet encoding routine */
/* Encodes a 3 byte mouse packet that complies with Microsoft Mouse/Logitech protocol */
void encodePacket(int x, int y, bool lb, bool rb, unsigned char* output)
{  
  /* Cap values just in case to avoid overflow errors */
  if ( x > 127 )
  {
    x = 127;
  }
  else if ( x < -127 )
  {
    x = -127;
  }
  
  if ( y > 127 )
  {
    y = 127;
  }
  else if ( y < -127 )
  {
    y = -127;
  }
  
  char cx = x;
  char cy = y;
    
  /* Packet 0 */
  output[0] = ((cx>>6)&0x3) | /* last 2 bits of X */
    (((cy>>6)&0x3)<<2) |      /* Last 2 bits of Y */
    (rb<<4)|(lb<<5)|0x40;     /* Mouse buttons and start packet bit */

  output[1] = cx&0x3f;        /* Packet 1 ( first 6 bits of X ) */
  output[2] = cy&0x3f;        /* Packet 2 ( first 6 bits of Y ) */
}


void setup()
{
  /* Set pin for RTS probe as input */
  pinMode( RTS_PROBE, INPUT );

  /* For blinking L13 LED */
  pinMode( 13, OUTPUT );
  digitalWrite( 13, LOW );
  
  #ifdef DEBUG
  /* Init serial for debug messages */
  Serial.begin( 115200 );
  
  while( !Serial )
  {
    /* Wait for USB serial to be ready (monitor active) */
  }
  
  Serial.print( "Init mouse\n" );
  #endif
  
  /* Initialize PS/2 mouse */
  mouse.initialize();
  mouse.set_sample_rate( 200 );
  mouse.set_scaling_1_1();
  
  /* Initialize serial for mouse data */
  Serial1.begin( 1200, SERIAL_7N1 );

  #ifdef DEBUG
  Serial.print("Program start!\n");
  #endif

  /* Flash LED to indicate adapter is ready */
  delay( 500 );
  for ( int i=0; i<4; i++ )
  {
    digitalWrite( 13, HIGH );
    delay( 200 );
    digitalWrite( 13, LOW );
    delay( 200 );
  }
}


void loop()
{
  short event = false;
  short event_mb = false;

  short left_changed = false;
  short right_changed = false;
  short middle_changed = false;
  
  int data[2];
  
  /* Read mouse data 4 times as PS/2 is too fast for serial */
  for(int i=0; i<4; i++)
  {
    mouse.report( data );
    
    x_status += data[1];
    y_status += -data[2];

    /* Check mouse events */
    
    if ( !left_changed )
    {
      if ( data[0] & 0x1 ) /* Left mouse button */
      {
        if ( !left_status )
        {
          event = true;
          left_status = true;
          left_changed = true;
        }
      }
      else
      {
        if ( left_status )
        { 
          event = true;
          left_status = false;
          left_changed = true;
        }
      }
    }

    if ( !right_changed )
    {
      if ( data[0] & 0x2 )  /* Right mouse button */
      {
        if ( !right_status )
        {
          event = true;
          right_status = true;
          right_changed = true;
        }
      }
      else
      {
        if ( right_status )
        { 
          event = true;
          right_status = false;
          right_changed = true;
        }
      }
    }

    if ( !middle_changed )
    {
      if ( data[0] & 0x4 )  /* Middle mouse button */
      {
        if (!middle_status)
        {
          event = true;
          event_mb = true;
          middle_status = true;
          middle_changed = true;
        }
        /* To compensate for the additional delay from sending 4 byte packets instead of 3 */
        mouse.report( data );
        mouse.report( data );
      }
      else
      {
        if ( middle_status )
        {
          event = true;
          event_mb = true;
          middle_status = false;
          middle_changed = true;
        }
      }
    }
    
  } 

  /* Divide velocity values to smoothen the movement */
  int x_status_d = x_status / 2;
  int y_status_d = y_status / 2;
  
  /* Reset X Y counters when divided result is non-zero */
  if ( x_status_d != 0 )
  {
    x_status = 0;
    event = true;
  }

  if ( y_status_d != 0 )
  {
    y_status = 0;
    event = true;
  }
  
  /* Send mouse events if there's any */
  if ( event )
  {
    #ifdef DEBUG
    Serial.print( "LB:" );
    Serial.print( left_status );
    Serial.print( " RB:" );
    Serial.print( right_status );
    Serial.print( " MB:" );
    Serial.print( middle_status );
    Serial.print( " X:" );
    Serial.print( x_status );
    Serial.print( " Y:" );
    Serial.print( y_status );
    Serial.println();
    #endif

    /* Encode the packet */
    unsigned char packet[4];
    int p_count = 3;
    
    encodePacket( x_status_d, y_status_d, 
      left_status, right_status, packet );

    /* Send extra byte for the middle mouse button status */
    if ( middle_status )
    {
      /* Keep sending 4th byte as long as middle button is down */
      #ifdef DEBUG
      Serial.print("MB is down.\n");
      #endif
      packet[3] = 0x20;
      p_count = 4;
    }
    else
    {
      /* 4th byte is sent once when middle button is lifted */
      if ( event_mb )
      {
        #ifdef DEBUG
        Serial.print("MB had been lifted.\n");
        #endif
        packet[3] = 0x0;
        p_count = 4;
      }
    }

    /* Send to PC via serial (not USB) */
    Serial1.write( packet, p_count );
  }
 
  /* Send init bytes when RTS has been toggled */
  if ( digitalRead( RTS_PROBE ) )
  {
    if ( !rts_status )
    {
      #ifdef DEBUG
      Serial.print("Send init byte!\n");
      #endif
      
      delay(14);
      Serial1.write( 'M' );
      delay(63);
      Serial1.write( '3' );
      
      left_status = false;
      right_status = false;
      middle_status = false;
      
      rts_status = true;
      digitalWrite( 13, HIGH );
      delay( 500 );
      digitalWrite( 13, LOW );
    }
  }
  else
  {
    rts_status = false;
  }
  
}
