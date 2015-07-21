/* RM */

#include "GATurnout.h"

namespace dev
{

GATurnout::GATurnout( int addr, uint8_t pin1, uint8_t pin2 )
{
  this->addr = addr;
  this->pin1 = pin1;
  this->pin2 = pin2;

  pinMode( pin1, OUTPUT );
  pinMode( pin2, OUTPUT );
  digitalWrite( pin1, LOW );
  digitalWrite( pin2, LOW );
}

int GATurnout::set( int addr, int port, int value, int _delay )
{
  int pin = port == 0 ? pin1 : pin2;
  if  (value != 0) {
    digitalWrite(pin, HIGH);
    delay(100);
    digitalWrite(pin, LOW);
  }

  return  ( 200 );
}


}

