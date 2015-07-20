/*
	SRCPServerSerial - SRCP Server welche Meldungen mittels
	der Seriellen Schnittstellen empfaengt und sendet.

	Siehe auch: http://srcpd.sourceforge.net/srcp/

	Copyright (c) 2013 Marcel Bernet.  All right reserved.

	[GNU General Public License]
 */

#ifndef SRCPSERVERSERIAL1_H_
#define SRCPSERVERSERIAL1_H_

#include <Arduino.h>
#include "SRCPSession.h"
#include "SRCPParser.h"

namespace srcp
{

class SRCPServerSerial1
{
private:
	SRCPSession _session;
	SRCPParser _parser;

	SRCPSession* session;
	SRCPParser* parser;
	srcp::command_t cmd;
public:
    void begin(unsigned long speed );
    command_t* dispatch( int fbDelay = 500 );
    // sendet direkt an die COMMAND Session
    void sendCommand( char* message ) { Serial.println( message ); }
    // sendet direkt an die INFO Session
    void sendInfo( char* message )  { Serial.println( message ); }
};

} /* namespace srcp */
#endif /* SRCPSERVERSERIAL1_H_ */
