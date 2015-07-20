/* RM */
#ifndef GATURNOUT_H_
#define GATURNOUT_H_

#include <inttypes.h>
#include <Arduino.h>
#include "SRCPGenericAccessoire.h"

namespace dev
{

class GATurnout : public srcp::SRCPGenericAccessoire
{
private:
  uint8_t pin1;
  uint8_t pin2;
public:
  GATurnout ( int addr, uint8_t start, uint8_t end );
  int get( int addr, int port ) { return ( 200 ); }
  int set( int addr, int port, int value, int delay );
};

}

#endif /* GATURNOUT_H_ */

