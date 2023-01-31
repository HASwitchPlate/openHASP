/*
 * FtpServer Arduino, esp8266 and esp32 library for Ftp Server
 * Derived form Jean-Michel Gallego version
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 *
 * Use Ethernet library
 * 
 * Commands implemented: 
 *   USER, PASS, AUTH (AUTH only return 'not implemented' code)
 *   CDUP, CWD, PWD, QUIT, NOOP
 *   MODE, PASV, PORT, STRU, TYPE
 *   ABOR, DELE, LIST, NLST, MLST, MLSD
 *   APPE, RETR, STOR
 *   MKD,  RMD
 *   RNTO, RNFR
 *   MDTM, MFMT
 *   FEAT, SIZE
 *   SITE FREE
 *   HELP
 *
 * Tested with those clients:
 *   under Windows:
 *     FTP Rush
 *     Filezilla
 *     WinSCP
 *     NcFTP, ncftpget, ncftpput
 *     Firefox
 *     command line ftp.exe
 *   under Ubuntu:
 *     gFTP
 *     Filezilla
 *     NcFTP, ncftpget, ncftpput
 *     lftp
 *     ftp
 *     Firefox
 *   under Android:
 *     AndFTP
 *     FTP Express
 *     Firefox
 *   with a second Arduino and sketch of SurferTim at
 *     http://playground.arduino.cc/Code/FTP
 * 
 */

#include <FtpServer.h>

FtpServer::FtpServer( uint16_t _cmdPort, uint16_t _pasvPort )
         : ftpServer( _cmdPort ), dataServer( _pasvPort )
{
  cmdPort = _cmdPort;
  pasvPort = _pasvPort;

  millisDelay = 0;
  nbMatch = 0;
  iCL = 0;

  iniVariables();
}

void FtpServer::begin( const char * _user, const char * _pass, const char * _welcomeMessage )
{
	if ( strcmp( _user, "anonymous" ) != 0) {
		DEBUG_PRINTLN(F("NOT ANONYMOUS"));
		DEBUG_PRINTLN(_user);
		this->anonymousConnection = false; // needed to reset after end of anonymnous and begin of not anonymous
	} else {
    this->anonymousConnection = true; 
  }
  // Tells the ftp server to begin listening for incoming connection
  ftpServer.begin();
  #if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040) || FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_SEEED_RTL8720DN
  ftpServer.setNoDelay( true );
  #endif
//  localIp = _localIP == FTP_NULLIP() || (uint32_t) _localIP == 0 ? NET_CLASS.localIP() : _localIP ;
  localIp = NET_CLASS.localIP(); //_localIP == FTP_NULLIP() || (uint32_t) _localIP == 0 ? NET_CLASS.localIP() : _localIP ;
//  strcpy( user, FTP_USER );
//  strcpy( pass, FTP_PASS );
  if( strlen( _user ) > 0 && strlen( _user ) < FTP_CRED_SIZE ) {
    //strcpy( user, _user );
	  this->user = _user;
  }
  if( strlen( _pass ) > 0 && strlen( _pass ) < FTP_CRED_SIZE ) {
//    strcpy( pass, _pass );
	  this->pass = _pass;
  }
//  strcpy(_welcomeMessage, welcomeMessage);

  this->welcomeMessage = _welcomeMessage;

  dataServer.begin();
#if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040) || FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_SEEED_RTL8720DN
  dataServer.setNoDelay( true );
#endif

  millisDelay = 0;
  cmdStage = FTP_Stop;
  iniVariables();
}

void FtpServer::begin( const char * _welcomeMessage ) {
	this->anonymousConnection = true;
	this->begin( "anonymous", "anonymous", _welcomeMessage);
}

void FtpServer::end()
{
    if(client.connected()) {
        disconnectClient();
    }

#if FTP_SERVER_NETWORK_TYPE == NETWORK_ESP32 && !defined(ARDUINO_ARCH_RP2040)
    ftpServer.end();
    dataServer.end();
#endif

    DEBUG_PRINTLN(F("Stop server!"));

    if (FtpServer::_callback) {
  	  FtpServer::_callback(FTP_DISCONNECT, free(), capacity());
    }

    cmdStage = FTP_Init;
    transferStage = FTP_Close;
    dataConn = FTP_NoConn;
}
void FtpServer::setLocalIp(IPAddress localIp)
{
	this->localIp = localIp;
}
void FtpServer::credentials( const char * _user, const char * _pass )
{
  if( strlen( _user ) > 0 && strlen( _user ) < FTP_CRED_SIZE )
//    strcpy( user, _user );
	  this->user = user;
  if( strlen( _pass ) > 0 && strlen( _pass ) < FTP_CRED_SIZE )
//    strcpy( pass, _pass );
	  this->pass = _pass;
}

void FtpServer::iniVariables()
{
  // Default for data port
  dataPort = FTP_DATA_PORT_DFLT;
  
  // Default Data connection is Active
  dataConn = FTP_NoConn;
  
  // Set the root directory
  strcpy( cwdName, "/" );

  rnfrCmd = false;
  transferStage = FTP_Close;
}

uint8_t FtpServer::handleFTP() {
#ifdef FTP_ADDITIONAL_DEBUG
//    int8_t data0 = data.status();
	ftpTransfer transferStage0 = transferStage;
	ftpCmd cmdStage0 = cmdStage;
	ftpDataConn dataConn0 = dataConn;
#endif

	if ((int32_t) (millisDelay - millis()) <= 0) {
		if (cmdStage == FTP_Stop) {
			if (client.connected()) {
				// DEBUG_PRINTLN(F("Disconnect client!"));
				disconnectClient();
			}
			cmdStage = FTP_Init;
		} else if (cmdStage == FTP_Init)  {  // Ftp server waiting for connection
			abortTransfer();
			iniVariables();
			DEBUG_PRINT(F("Server waiting for connection on port %d"),cmdPort);
			// DEBUG_PRINTLN(cmdPort);

			cmdStage = FTP_Client;
		} else if (cmdStage == FTP_Client) {    // Ftp server idle
#if (FTP_SERVER_NETWORK_TYPE == NETWORK_WiFiNINA)
//			if (client && !client.connected()) {
//				client.stop();
//				DEBUG_PRINTLN(F("CLIENT STOP!!"));
//			}
			byte status;
			client = ftpServer.available(&status);
			/*
			 *   CLOSED      = 0,
  LISTEN      = 1,
  SYN_SENT    = 2,
  SYN_RCVD    = 3,
  ESTABLISHED = 4,
  FIN_WAIT_1  = 5,
  FIN_WAIT_2  = 6,
  CLOSE_WAIT  = 7,
  CLOSING     = 8,
  LAST_ACK    = 9,
  TIME_WAIT   = 10
			 *
			 */
//			DEBUG_PRINTLN(status);
#elif defined(ESP8266) // || defined(ARDUINO_ARCH_RP2040)
		  if( ftpServer.hasClient())
		  {
		    client.stop();
		    client = ftpServer.available();
		  }
#else
			if (client && !client.connected()) {
				client.stop();
				DEBUG_PRINTLN(F("CLIENT STOP!!"));
			}
			client = ftpServer.accept();
#endif
			if (client.connected())             // A client connected
			{
				clientConnected();
				millisEndConnection = millis() + 1000L * FTP_AUTH_TIME_OUT; // wait client id for 10 s.
				cmdStage = FTP_User;
			}
		} else if (readChar() > 0)             // got response
				{
			processCommand();
			if (cmdStage == FTP_Stop)
				millisEndConnection = millis() + 1000L * FTP_AUTH_TIME_OUT; // wait authentication for 10 s.
			else if (cmdStage < FTP_Cmd)
				millisDelay = millis() + 200;     // delay of 100 ms
			else
				millisEndConnection = millis() + 1000L * FTP_TIME_OUT;
		} else if (!client.connected()) {
			if (FtpServer::_callback) {
			  FtpServer::_callback(FTP_DISCONNECT, free(), capacity());
			}

			cmdStage = FTP_Init;
		}
		if (transferStage == FTP_Retrieve)   // Retrieve data
				{
			if (!doRetrieve()) {
				transferStage = FTP_Close;
			}
		} else if (transferStage == FTP_Store) // Store data
		{
			if (!doStore()) {
		    	  if (FtpServer::_callback) {
		    		  FtpServer::_callback(FTP_FREE_SPACE_CHANGE, free(), capacity());
		    	  }

				transferStage = FTP_Close;
			}
		} else if (transferStage == FTP_List || transferStage == FTP_Nlst) // LIST or NLST
				{
			if (!doList()) {
				transferStage = FTP_Close;
			}
		} else if (transferStage == FTP_Mlsd)  // MLSD listing
				{
			if (!doMlsd()) {

				transferStage = FTP_Close;
			}
		} else if (cmdStage > FTP_Client
				&& !((int32_t) (millisEndConnection - millis()) > 0)) {
			client.println(F("530 Timeout"));
			millisDelay = millis() + 200;       // delay of 200 ms
			cmdStage = FTP_Stop;
		}

#ifdef FTP_ADDITIONAL_DEBUG
		if (cmdStage != cmdStage0 || transferStage != transferStage0
				|| dataConn != dataConn0) {
			DEBUG_PRINT(F("  Command Old: "));
			DEBUG_PRINT(cmdStage0);
			DEBUG_PRINT(F("  Transfer Old: "));
			DEBUG_PRINT(transferStage0);
			DEBUG_PRINT(F("  Data Old: "));
			DEBUG_PRINTLN(dataConn0);

			DEBUG_PRINT(F("  Command    : "));
			DEBUG_PRINT(cmdStage);
			DEBUG_PRINT(F("  Transfer    : "));
			DEBUG_PRINT(transferStage);
			DEBUG_PRINT(F("  Data    : "));
			DEBUG_PRINTLN(dataConn);
		}
#endif
	}
	return cmdStage | (transferStage << 3) | (dataConn << 6);
}

void FtpServer::clientConnected()
{
  // DEBUG_PRINTLN( F(" Client connected!") );
  client.print(F("220 ---")); client.print(welcomeMessage); client.println(F(" ---"));
  // client.print(F("220---")); client.print(welcomeMessage); client.println(F(" ---"));
  // client.println(F("220---   By Renzo Mischianti   ---"));
  // client.print(F("220 --    Version ")); client.print(FTP_SERVER_VERSION); client.println(F("    --"));
  iCL = 0;
  if (FtpServer::_callback) {
	  FtpServer::_callback(FTP_CONNECT, free(), capacity());
  }

}

void FtpServer::disconnectClient()
{
	// DEBUG_PRINTLN( F(" Disconnecting client") );

  abortTransfer();
  client.println(F("221 Goodbye") );

  if (FtpServer::_callback) {
	  FtpServer::_callback(FTP_DISCONNECT, free(), capacity());
  }

  if( client ) {
  }
  if( data ) {
    data.stop();
  }
}

bool FtpServer::processCommand()
{
  ///////////////////////////////////////
  //                                   //
  //      AUTHENTICATION COMMANDS      //
  //                                   //
  ///////////////////////////////////////

  // RoSchmi added the next two lines
  // DEBUG_PRINT("Command is: ");
  //DEBUG_PRINTLN(command);

  //
  //  USER - User Identity 
  //
  if( CommandIs( "USER" ))
  {
	  DEBUG_PRINT(F("USER: %s"),parameter);
	  // DEBUG_PRINT(parameter);
	  // DEBUG_PRINT(F(" "));
	  // DEBUG_PRINTLN(user)

	if (this->anonymousConnection &&  ! strcmp( parameter, user )) {
    	DEBUG_PRINTLN( F("Anonymous authentication Ok. Waiting for commands.") );

      client.println(F("230 Ok") );
      cmdStage = FTP_Cmd;
	} else if( ! strcmp( parameter, user ))
    {
      client.println(F("331 Ok. Password required") );
      strcpy( cwdName, "/" );
      cmdStage = FTP_Pass;
    }
    else
    {
      client.println(F("530 ") );
      cmdStage = FTP_Stop;
    }
  }
  //
  //  PASS - Password
  //
  else if( CommandIs( "PASS" ))
  {
    if( cmdStage != FTP_Pass )
    {
      client.println(F("503 ") );
      cmdStage = FTP_Stop;
    }
    if( ! strcmp( parameter, pass ))
    {
    	DEBUG_PRINTLN( F("Authentication Ok. Waiting for commands.") );

      client.println(F("230 Ok") );
      cmdStage = FTP_Cmd;
    }
    else
    {
    	client.println( F("530 ") );
      cmdStage = FTP_Stop;
    }
  }
  //
  //  FEAT - New Features
  //
  else if( CommandIs( "FEAT" ))
  {
    client.println(F("211-Extensions suported:"));
    client.println(F(" MLST type*;modify*;size*;") );
    client.println(F(" MLSD") );
    client.println(F(" MDTM") );
    client.println(F(" MFMT") );
#ifdef UTF8_SUPPORT
	client.println(F(" UTF8") );
#endif
    client.println(F(" SIZE") );
    client.println(F(" SITE FREE") );
    client.println(F("211 End.") );
  }
  //
  //  AUTH - Not implemented
  //
  else if( CommandIs( "AUTH" ))
    client.println(F("502 ") );
  //
  //  Unrecognized commands at stage of authentication
  //
  else if( cmdStage < FTP_Cmd )
  {
    client.println(F("530 ") );
    cmdStage = FTP_Stop;
  }

  ///////////////////////////////////////
  //                                   //
  //      ACCESS CONTROL COMMANDS      //
  //                                   //
  ///////////////////////////////////////

  //
  //  PWD - Print Directory
  //
  else if( CommandIs( "PWD" ) ||
           ( CommandIs( "CWD" ) && ParameterIs( "." ))) {
	  client.print( F("257 \"")); client.print( cwdName ); client.print( F("\"") ); client.println( F(" is your current directory") );
  //
  //  CDUP - Change to Parent Directory 
  //
  } else if( CommandIs( "CDUP" ) ||
           ( CommandIs( "CWD" ) && ParameterIs( ".." )))
  {
    bool ok = false;
    
    if( strlen( cwdName ) > 1 )            // do nothing if cwdName is root
    {
      // if cwdName ends with '/', remove it (must not append)
      if( cwdName[ strlen( cwdName ) - 1 ] == '/' ) {
        cwdName[ strlen( cwdName ) - 1 ] = 0;
      }
      // search last '/'
      char * pSep = strrchr( cwdName, '/' );
      ok = pSep > cwdName;
      // if found, ends the string on its position
      if( ok )
      {
        * pSep = 0;
        ok = exists( cwdName );
      }
    }
    // if an error appends, move to root
    if( ! ok ) {
      strcpy( cwdName, "/" );
    }
    client.print( F("250 Ok. Current directory is ") ); client.println( cwdName );
  }
  //
  //  CWD - Change Working Directory
  //
  else if( CommandIs( "CWD" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path ))
    {
      strcpy( cwdName, path );
      client.print( F("250 Directory changed to ") ); client.print(cwdName); client.println();
    }
  }
  //
  //  QUIT
  //
  else if( CommandIs( "QUIT" ))
  {
    client.println(F("221 Goodbye") );
    disconnectClient();
    cmdStage = FTP_Stop;
  }

  ///////////////////////////////////////
  //                                   //
  //    TRANSFER PARAMETER COMMANDS    //
  //                                   //
  ///////////////////////////////////////

  //
  //  MODE - Transfer Mode 
  //
  else if( CommandIs( "MODE" ))
  {
    if( ParameterIs( "S" )) {
      client.println(F("200 S Ok") );
    } else {
      client.println(F("504 Only S(tream) is suported") );
    }
  }
  //
  //  PASV - Passive Connection management
  //
  else if( CommandIs( "PASV" ))
  {
    data.stop();
    dataServer.begin();
    if (((((uint32_t) NET_CLASS.localIP()) & ((uint32_t) NET_CLASS.subnetMask())) ==
       (((uint32_t) client.remoteIP()) & ((uint32_t) NET_CLASS.subnetMask()))) && (uint32_t)localIp <= 0) {
      dataIp = NET_CLASS.localIP();
    } else {
      dataIp = localIp;
    }
    // DEBUG_PRINT( F(" IP: %s"), dataIp.toString().c_str() );
    // DEBUG_PRINT( int( dataIp[0]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[1]) ); DEBUG_PRINT( F(".") );
    // DEBUG_PRINT( int( dataIp[2]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINTLN( int( dataIp[3]) );

#if !defined(ARDUINO_ARCH_RP2040) && ((FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_ESP8266_ASYNC) || (FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_ESP8266) || (FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_ESP8266) || (FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_ESP32)) // || 	(FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_WiFiNINA)  || (FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_SEEED_RTL8720DN))
    if (dataIp.toString() ==  F("0.0.0.0")) {
    	dataIp = NET_CLASS.softAPIP();
    }
#endif
  //  DEBUG_PRINT( F(" Soft IP: %s"), dataIp.toString().c_str());
	// DEBUG_PRINT( int( dataIp[0]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[1]) ); DEBUG_PRINT( F(".") );
	// DEBUG_PRINT( int( dataIp[2]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINTLN( int( dataIp[3]) );

    dataPort = pasvPort;
    // DEBUG_PRINTLN( F(" Connection management set to passive") );
    DEBUG_PRINT( F("Listening at %s:%d"),dataIp.toString().c_str(),dataPort );
    // DEBUG_PRINT( int( dataIp[0]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[1]) ); DEBUG_PRINT( F(".") );
    // DEBUG_PRINT( int( dataIp[2]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[3]) );
    // DEBUG_PRINT( F(":") ); DEBUG_PRINTLN( dataPort );

    client.print( F("227 Entering Passive Mode") ); client.print( F(" (") );
    client.print( int( dataIp[0]) ); client.print( F(",") ); client.print( int( dataIp[1]) ); client.print( F(",") );
    client.print( int( dataIp[2]) ); client.print( F(",") ); client.print( int( dataIp[3]) ); client.print( F(",") );
    client.print( ( dataPort >> 8 ) ); client.print( F(",") ); client.print( ( dataPort & 255 ) ); client.println( F(")") );
    dataConn = FTP_Pasive;
  }
  //
  //  PORT - Data Port
  //
  else if( CommandIs( "PORT" ))
  {
    data.stop();
    // get IP of data client
    dataIp[ 0 ] = atoi( parameter );
    char * p = strchr( parameter, ',' );
    for( uint8_t i = 1; i < 4; i ++ )
    {
      dataIp[ i ] = atoi( ++ p );
      p = strchr( p, ',' );
    }
    // get port of data client
    dataPort = 256 * atoi( ++ p );
    p = strchr( p, ',' );
    dataPort += atoi( ++ p );
    if( p == NULL ) {
      client.println(F("501 Can't interpret parameters") );
    } else
    {
    	// DEBUG_PRINT( F(" Data IP set to ") ); DEBUG_PRINT( int( dataIp[0]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[1]) );
    	// DEBUG_PRINT( F(".") ); DEBUG_PRINT( int( dataIp[2]) ); DEBUG_PRINT( F(".") ); DEBUG_PRINTLN( int( dataIp[3]) );
    	// DEBUG_PRINT( F(" Data port set to ") ); DEBUG_PRINTLN( dataPort );

    	DEBUG_PRINT( F("Remote endpoint %s:%d"), dataIp.toString().c_str(), dataPort );


      client.println(F("200 PORT command successful") );
      dataConn = FTP_Active;
    }
  }
  //
  //  STRU - File Structure
  //
  else if( CommandIs( "STRU" ))
  {
    if( ParameterIs( "F" )) {
      client.println(F("200 F Ok") );
    // else if( ParameterIs( "R" ))
    //  client.println(F("200 B Ok") );
    }else{
      client.println(F("504 Only F(ile) is suported") );
    }
  }
  //
  //  TYPE - Data Type
  //
  else if( CommandIs( "TYPE" ))
  {
    if( ParameterIs( "A" )) {
      client.println(F("200 TYPE is now ASCII"));
    } else if( ParameterIs( "I" )) {
      client.println(F("200 TYPE is now 8-bit binary") );
    } else {
      client.println(F("504 Unknow TYPE") );
    }
  }

  ///////////////////////////////////////
  //                                   //
  //        FTP SERVICE COMMANDS       //
  //                                   //
  ///////////////////////////////////////

  //
  //  ABOR - Abort
  //
  else if( CommandIs( "ABOR" ))
  {
    abortTransfer();
    client.println(F("226 Data connection closed"));
  }
  //
  //  DELE - Delete a File 
  //
  else if( CommandIs( "DELE" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path )) {
      if( remove( path )) {
    	  if (FtpServer::_callback) {
    		  FtpServer::_callback(FTP_FREE_SPACE_CHANGE, free(), capacity());
    	  }

        client.print( F("250 Deleted ") ); client.println( parameter );
      } else {
    	  client.print( F("450 Can't delete ") ); client.println( parameter );
      }
    }
  }
  //
  //  LIST - List
  //  NLST - Name List
  //  MLSD - Listing for Machine Processing (see RFC 3659)
  //
  else if( CommandIs( "LIST" ) || CommandIs( "NLST" ) || CommandIs( "MLSD" ))
  {
	  //DEBUG_PRINT("List of file!!");

    if( dataConnect()){
      if( openDir( & dir ))
      {
    	// DEBUG_PRINT("Dir opened!!");

        nbMatch = 0;
        if( CommandIs( "LIST" ))
          transferStage = FTP_List;
        else if( CommandIs( "NLST" ))
          transferStage = FTP_Nlst;
        else
          transferStage = FTP_Mlsd;
      }
      else {
    	  DEBUG_PRINT("List Data stop!!");
    	  data.stop();
      }
    }
  }
  //
  //  MLST - Listing for Machine Processing (see RFC 3659)
  //
  else if( CommandIs( "MLST" ))
  {
    char path[ FTP_CWD_SIZE ];
    uint16_t dat=0, tim=0;
    char dtStr[ 15 ];
    bool isdir;
    if( haveParameter() && makeExistsPath( path )){
      if( ! getFileModTime( path, &dat, &tim )) {
        client.print( F("550 Unable to retrieve time for ") ); client.println( parameter );
      } else
      {
        isdir = isDir( path );
        client.println( F("250-Begin") );
        client.print( F(" Type=") ); client.print( ( isdir ? F("dir") : F("file")) );
        client.print( F(";Modify=") ); client.print( makeDateTimeStr( dtStr, dat, tim ) );
        if( ! isdir )
        {
          if( openFile( path, FTP_FILE_READ ))
          {
            client.print( F(";Size=") ); client.print( long( fileSize( file )) );
            file.close();
          }
        }
        client.print( F("; ") ); client.println( path );
        client.println( F("250 End.") );
      }
    }
  }
  //
  //  NOOP
  //
  else if( CommandIs( "NOOP" )) {
    client.println(F("200 Zzz...") );
  }
  //
#ifdef UTF8_SUPPORT
  //  OPTS
  //
  else if( CommandIs( "OPTS" )) {
    if( ParameterIs( "UTF8 ON" ) || ParameterIs( "utf8 on" )) {
      client.println(F("200 OK, UTF8 ON") );
      // DEBUG_PRINTLN(F("200 OK, UTF8 ON") );
    } else {
      client.println(F("504 Unknow OPTS") );
      DEBUG_PRINTLN(F("504 Unknow OPTS") );
    }
  }
  //
#endif
  //  HELP
  //
  else if( CommandIs( "HELP" )) {
    client.println(F("200 Commands implemented:") );
	client.println(F("      USER, PASS, AUTH (AUTH only return 'not implemented' code)") );
	client.println(F("      CDUP, CWD, PWD, QUIT, NOOP") );
	client.println(F("      MODE, PASV, PORT, STRU, TYPE") );
	client.println(F("      ABOR, DELE, LIST, NLST, MLST, MLSD") );
	client.println(F("      APPE, RETR, STOR") );
	client.println(F("      MKD,  RMD") );
	client.println(F("      RNTO, RNFR") );
	client.println(F("      MDTM, MFMT") );
	client.println(F("      FEAT, SIZE") );
	client.println(F("      SITE FREE") );
	client.println(F("      HELP") );
  }
  //
  //  RETR - Retrieve
  //
  else if( CommandIs( "RETR" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path )) {
      if( ! openFile( path, FTP_FILE_READ )) {
        client.print( F("450 Can't open ") ); client.print( parameter );
      } else if( dataConnect( false ))
      {
    	//  DEBUG_PRINT( F(" Sending ") ); DEBUG_PRINT( parameter ); DEBUG_PRINT( F(" size ") ); DEBUG_PRINTLN( long( fileSize( file ))  );
    	//  DEBUG_PRINT( F(" Sending %s size %d"), parameter, long( fileSize( file ))  );

		  if (FtpServer::_transferCallback) {
			  FtpServer::_transferCallback(FTP_DOWNLOAD_START, parameter,  long( fileSize( file )));
		  }


        client.print( F("150-Connected to port ") ); client.println( dataPort );
        client.print( F("150 ") ); client.print( long( fileSize( file )) ); client.println( F(" bytes to download") );
        millisBeginTrans = millis();
        bytesTransfered = 0;
        transferStage = FTP_Retrieve;
      }
    }
  }
  //
  //  STOR - Store
  //  APPE - Append
  //
  else if( CommandIs( "STOR" ) || CommandIs( "APPE" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makePath( path ))
    {
      bool open;
      if( exists( path )) {
    	  DEBUG_PRINTLN(F("APPEND FILE!!"));
        open = openFile( path, ( CommandIs( "APPE" ) ? FTP_FILE_WRITE_APPEND : FTP_FILE_WRITE_CREATE ));
      } else {
    	  DEBUG_PRINTLN(F("CREATE FILE!!"));
        open = openFile( path, FTP_FILE_WRITE_CREATE );
      }

      data.stop();
      data.flush();

      // DEBUG_PRINT(F("open/create "));
      // DEBUG_PRINTLN(open);
      if( ! open ){
    	  client.print( F("451 Can't open/create ") ); client.println( parameter );
      }else if( ! dataConnect()) // && !data.available())
        file.close();
      else
      {
    	  // DEBUG_PRINT( F(" Receiving %s") , parameter );

        millisBeginTrans = millis();
        bytesTransfered = 0;
        transferStage = FTP_Store;

		  if (FtpServer::_transferCallback) {

			  FtpServer::_transferCallback(FTP_UPLOAD_START, parameter, bytesTransfered);
		  }

      }
    }
  }
  //
  //  MKD - Make Directory
  //
  else if( CommandIs( "MKD" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makePath( path ))
    {
      if( exists( path )) {
    	  client.print( F("521 \"") ); client.print( parameter ); client.println( F("\" directory already exists") );
      } else
      {
    	  DEBUG_PRINT( F(" Creating directory %s"), parameter );

#if STORAGE_TYPE != STORAGE_SPIFFS
        if( makeDir( path )) {
        	client.print( F("257 \"") ); client.print( parameter ); client.print( F("\"") ); client.println( F(" created") );
        } else {
#endif
        	client.print( F("550 Can't create \"") ); client.print( parameter ); client.println( F("\"") );
#if STORAGE_TYPE != STORAGE_SPIFFS
        }
#endif
      }
    }
  }
  //
  //  RMD - Remove a Directory 
  //
  else if( CommandIs( "RMD" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path )) {
      if( removeDir( path ))
      {
    	  DEBUG_PRINT( F(" Deleting %s"), path );

    	  client.print( F("250 \"") ); client.print( parameter ); client.println( F("\" deleted") );
      }
      else {
    	  client.print( F("550 Can't remove \"") ); client.print( parameter ); client.println( F("\". Directory not empty?") );
      }
    }
  }
  //
  //  RNFR - Rename From 
  //
  else if( CommandIs( "RNFR" ))
  {
    rnfrName[ 0 ] = 0;
    if( haveParameter() && makeExistsPath( rnfrName ))
    {
    	DEBUG_PRINT( F(" Ready for renaming ") ); DEBUG_PRINTLN( rnfrName );

      client.println(F("350 RNFR accepted - file exists, ready for destination") );
      rnfrCmd = true;
    }
  }
  //
  //  RNTO - Rename To 
  //
  else if( CommandIs( "RNTO" ))
  {
    char path[ FTP_CWD_SIZE ];
    char dirp[ FTP_FIL_SIZE ];
    if( strlen( rnfrName ) == 0 || ! rnfrCmd ) {
      client.println(F("503 Need RNFR before RNTO") );
    } else if( haveParameter() && makePath( path ))
    {
      if( exists( path )) {
        client.print( F("553 ") ); client.print( parameter ); client.println( F(" already exists") );
      } else
      {
        strcpy( dirp, path );
        char * psep = strrchr( dirp, '/' );
        bool fail = psep == NULL;
        if( ! fail )
        {
          if( psep == dirp )
            psep ++;
          * psep = 0;
//          fail = ! isDir( dirp );
//          if( fail ) {
//        	  client.print( F("550 \"") ); client.print( dirp ); client.println( F("\" is not directory") );
//          } else
//          {
        	  DEBUG_PRINT( F(" Renaming ") ); DEBUG_PRINT( rnfrName ); DEBUG_PRINT( F(" to ") ); DEBUG_PRINTLN( path );

            if( rename( rnfrName, path ))
              client.println(F("250 File successfully renamed or moved") );
            else
              fail = true;
//          }
        }
        if( fail )
          client.println(F("451 Rename/move failure") );
      }
    }
    rnfrCmd = false;
  }
  /*
  //
  //  SYST - System
  //
  else if( CommandIs( "SYST" ))
    FtpOutCli << F("215 MSDOS") << endl;
  */
  
  ///////////////////////////////////////
  //                                   //
  //   EXTENSIONS COMMANDS (RFC 3659)  //
  //                                   //
  ///////////////////////////////////////

  //
  //  MDTM && MFMT - File Modification Time (see RFC 3659)
  //
  else if( CommandIs( "MDTM" ) || CommandIs( "MFMT" ))
  {
    if( haveParameter())
    {
      char path[ FTP_CWD_SIZE ];
      char * fname = parameter;
      uint16_t year;
      uint8_t month, day, hour, minute, second, setTime;
      char dt[ 15 ];
      bool mdtm = CommandIs( "MDTM" );

      setTime = getDateTime( dt, & year, & month, & day, & hour, & minute, & second );
      // fname point to file name
      fname += setTime;
      if( strlen( fname ) <= 0 ) {
        client.println(F("501 No file name") );
      } else if( makeExistsPath( path, fname )) {
        if( setTime ) // set file modification time
        {
          if( timeStamp( path, year, month, day, hour, minute, second )) {
            client.print( F("213 ") ); client.println( dt );
          } else {
            client.println(F("550 Unable to modify time" ));
          }
        }
        else if( mdtm ) // get file modification time
        {
          uint16_t dat=0, tim=0;
          char dtStr[ 15 ];
          if( getFileModTime( path, &dat, &tim )) {
            client.print( F("213 ") ); client.println( makeDateTimeStr( dtStr, dat, tim ) );
          } else {
            client.println("550 Unable to retrieve time" );
          }
        }
      }
    }
  }
  //
  //  SIZE - Size of the file
  //
  else if( CommandIs( "SIZE" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( haveParameter() && makeExistsPath( path )) {
      if( ! openFile( path, FTP_FILE_READ )) {
        client.print( F("450 Can't open ") ); client.println( parameter );
      } else
      {
        client.print( F("213 ") ); client.println( long( fileSize( file )) );
        file.close();
      }
    }
  }
  //
  //  SITE - System command
  //
  else if( CommandIs( "SITE" ))
  {
    if( ParameterIs( "FREE" ))
    {
      uint32_t capa = capacity();
      if(( capa >> 10 ) < 1000 ) { // less than 1 Giga
        client.print( F("200 ") ); client.print( free() ); client.print( F(" kB free of ") );
        client.print( capa ); client.println( F(" kB capacity") );
      }else {
    	  client.print( F("200 ") ); client.print( ( free() >> 10 ) ); client.print( F(" MB free of ") );
    	  client.print( ( capa >> 10 ) ); client.println( F(" MB capacity") );
      }
    }
    else {
    	client.print( F("500 Unknow SITE command ") ); client.println( parameter );
    }
  }
  //
  //  Unrecognized commands ...
  //
  else
    client.println(F("500 Unknow command") );
  return true;
}

int FtpServer::dataConnect( bool out150 )
{
  if( ! data.connected()) {
    if( dataConn == FTP_Pasive )
    {
      uint16_t count = 1000; // wait up to a second
      while( ! data.connected() && count -- > 0 )
      {
		#if (FTP_SERVER_NETWORK_TYPE == NETWORK_WiFiNINA)
    	  	  data = dataServer.available();
		#elif defined(ESP8266) // || defined(ARDUINO_ARCH_RP2040)
			if( dataServer.hasClient())
			{
			  data.stop();
			  data = dataServer.available();
			}
        #else
			data = dataServer.accept();
        #endif
        delay( 1 );
      }
    }
    else if( dataConn == FTP_Active )
      data.connect( dataIp, dataPort );
  }

//#ifdef ESP8266
  if( ! ( data.connected() || data.available())) {
//#else
//	  if( ! ( data.connected() )) {
//#endif
    client.println(F("425 No data connection"));
  } else if( out150 ) {
    client.print( F("150 Accepted data connection to port ") ); client.println( dataPort );
  }
//#ifdef ESP8266
	  return  data.connected() || data.available();
//#else
//	  return  data.connected();
//#endif

}

bool FtpServer::dataConnected()
{
  if( data.connected())
    return true;
  data.stop();
  client.println(F("426 Data connection closed. Transfer aborted") );
  transferStage = FTP_Close;
  return false;
}
 
bool FtpServer::openDir( FTP_DIR * pdir )
{
  bool openD;
#if (STORAGE_TYPE == STORAGE_LITTLEFS && (defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)))
  if( cwdName == 0 ) {
    dir = STORAGE_MANAGER.openDir( "/" );
  } else {
    dir = STORAGE_MANAGER.openDir( cwdName );
  }
  openD = dir.rewind();

  if( ! openD ) {
    client.print( F("550 Can't open directory ") ); client.println( cwdName );
  }
#elif STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC
  if( cwdName == 0 ) {
	    dir = STORAGE_MANAGER.open( "/" );
	  } else {
	    dir = STORAGE_MANAGER.open( cwdName );
	  }
	  openD = true;
	  if( ! openD ) {
		client.print( F("550 Can't open directory ") ); client.println( cwdName );
	  }
#elif STORAGE_TYPE == STORAGE_FFAT || (STORAGE_TYPE == STORAGE_LITTLEFS && defined(ESP32))
	  if( cwdName == 0 ) {
	    dir = STORAGE_MANAGER.open( "/" );
	  } else {
	    dir = STORAGE_MANAGER.open( cwdName );
	  }
	  openD = true;
	  if( ! openD ) {
		client.print( F("550 Can't open directory ") ); client.println( cwdName );
	  }
#elif STORAGE_TYPE == STORAGE_SEEED_SD
	  if( cwdName == 0 ) {
	  	  DEBUG_PRINT("cwdName forced -> ");
	  	  DEBUG_PRINTLN(cwdName );

	  	  FTP_DIR d = STORAGE_MANAGER.open( "/" );
		  dir=d;
	  } else {
		  DEBUG_PRINT("cwdName -> ");
		  DEBUG_PRINTLN(cwdName );

		  FTP_DIR d = STORAGE_MANAGER.open( cwdName );
		  dir=d;
	  }

	  openD = dir.isDirectory();

	  if( ! openD  ) {
		client.print( F("550 Can't open directory ") ); client.println( cwdName );
	  }
#elif STORAGE_TYPE == STORAGE_SPIFFS
  if( cwdName == 0 || strcmp(cwdName, "/") == 0 ) {
	  DEBUG_PRINT("DIRECTORY / EXIST ");
#if ESP8266
	  dir = STORAGE_MANAGER.openDir( "/" );
#else
	  dir = STORAGE_MANAGER.open( "/" );
#endif
	  openD = true;

    } else {
    	openD = false;
    }
    if( ! openD ) {
      client.print( F("550 Can't open directory ") ); client.println( cwdName );
    }
#else
  if( cwdName == 0 ) {
    openD = pdir->open( "/" );
  } else {
    openD = pdir->open( cwdName );
  }
  if( ! openD ) {
    client.print( F("550 Can't open directory ") ); client.println( cwdName );
  }
#endif
  return openD;
}

bool FtpServer::doRetrieve()
{
  if( ! dataConnected())
  {
    file.close();
    return false;
  }
  int16_t nb = file.read( buf, FTP_BUF_SIZE );
  if( nb > 0 )
  {
    data.write( buf, nb );
    // DEBUG_PRINT(F("NB --> "));
    // DEBUG_PRINTLN(nb);
    bytesTransfered += nb;

	  if (FtpServer::_transferCallback) {
		  FtpServer::_transferCallback(FTP_DOWNLOAD, getFileName(&file), bytesTransfered);
	  }

// RoSchmi
#if STORAGE_TYPE != STORAGE_SEEED_SD
    return true;
#endif
  }
  closeTransfer();
  return false;
}

bool FtpServer::doStore()
{
  int16_t na = data.available();
  if( na == 0 ) {
	  // DEBUG_PRINTLN("NO DATA AVAILABLE!");
#if FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_SEEED_RTL8720DN
	  data.stop();
#endif
    if( data.connected()) {
      return true;
    } else
    {
      closeTransfer();
      return false;
    }
  }

  if( na > FTP_BUF_SIZE ) {
    na = FTP_BUF_SIZE;
  }
  int16_t nb = data.read((uint8_t *) buf, na );
  int16_t rc = 0;
  if( nb > 0 )
  {
	    // DEBUG_PRINT("NB -> ");
	    // DEBUG_PRINTLN(nb);

    rc = file.write( buf, nb );
    // DEBUG_PRINT("RC -> ");
    // DEBUG_PRINTLN(rc);
    bytesTransfered += nb;

	  if (FtpServer::_transferCallback) {

		  FtpServer::_transferCallback(FTP_UPLOAD, getFileName(&file), bytesTransfered);
	  }
  }
  if( nb < 0 || rc == nb  ) {
    return true;
  }
  client.println(F("552 Probably insufficient storage space") );
  file.close();
  data.stop();
  return false;
}

void generateFileLine(FTP_CLIENT_NETWORK_CLASS* data, bool isDirectory, const char* fn, long fz, const char* time, const char* user, bool writeFilename = true) {
	if( isDirectory ) {
		//			  data.print( F("+/,\t") );
		//			  DEBUG_PRINT(F("+/,\t"));

		data->print( F("drwxrwsr-x\t2\t"));
		data->print( user );
		data->print( F("\t") );
		data->print( long( 4096 ) );
		data->print( F("\t") );

		DEBUG_PRINT( F("drwxrwsr-x\t2\t") );
		DEBUG_PRINT( user );
		DEBUG_PRINT( F("\t") );

		// DEBUG_PRINT( long( 4096 ) );
		DEBUG_PRINT( F("\t") );

		data->print(time);
		DEBUG_PRINT(time);

		data->print( F("\t") );
		if (writeFilename) data->println( fn );

		DEBUG_PRINT( F("\t") );
		if (writeFilename) DEBUG_PRINTLN( fn );

	} else {
//			data.print( F("+r,s") );
//			DEBUG_PRINT(F("+r,s"));

		data->print( F("-rw-rw-r--\t1\t") );
		data->print( user );
		data->print( F("\t") );
		data->print( fz );
		data->print( F("\t") );

		DEBUG_PRINT( F("-rw-rw-r--\t1\t") );
		DEBUG_PRINT( user );
		DEBUG_PRINT( F("\t") );
		// DEBUG_PRINT( fz );
		DEBUG_PRINT( F("\t") );

		data->print(time);
		DEBUG_PRINT(time);

		data->print( F("\t") );
		if (writeFilename) data->println( fn );

		DEBUG_PRINT( F("\t") );
		if (writeFilename) DEBUG_PRINTLN( fn );
	}

}

#if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
//
// Formats printable String from a time_t timestamp
//
String makeDateTimeStrList(time_t ft, bool dateContracted = false)
{
  String tmp;
  // a buffer with enough space for the formats
  char buf[25];
  char *b = buf;

  // break down the provided file time
  struct tm _tm;
  gmtime_r(&ft, &_tm);

  if (dateContracted)
  {
    // "%Y%m%d%H%M%S", e.g. "20200517123400"
    strftime(b, sizeof(buf), "%Y%m%d%H%M%S", &_tm);
  }
  else
  {
    // "%h %d %H:%M", e.g. "May 17 12:34" for file dates of the current year
    // "%h %d  %Y"  , e.g. "May 17  2019" for file dates of any other years

    // just convert both ways, select later what's to be shown
    // buf becomes "May 17  2019May 17 12:34"
    strftime(b, sizeof(buf), "%h %d %H:%M%h %d  %Y", &_tm);

    // check for a year != year from now
    int fileYear = _tm.tm_year;
    time_t nowTime = time(NULL);
    gmtime_r(&nowTime, &_tm);
    if (fileYear == _tm.tm_year)
    {
      // cut off 2nd half - year variant
      b[12] = '\0';
    }
    else
    {
      // skip 1st half - time variant
      b += 12;
    }
  }
  tmp = b;
  return tmp;
}

// https://files.stairways.com/other/ftp-list-specs-info.txt
void generateFileLine(FTP_CLIENT_NETWORK_CLASS* data, bool isDirectory, const char* fn, long fz, time_t time, const char* user, bool writeFilename = true) {
	generateFileLine(data, isDirectory, fn, fz, makeDateTimeStrList(time).c_str(), user, writeFilename);
}
#endif

bool FtpServer::doList()
{
  if( ! dataConnected())
  {
#if STORAGE_TYPE != STORAGE_SPIFFS && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SEEED_SD
    dir.close();
#endif
    return false;
  }
#if STORAGE_TYPE == STORAGE_SPIFFS
	#if ESP8266
	  if( dir.next())
	#else
	  FTP_FILE fileDir = dir.openNextFile();
	  if( fileDir )
	#endif
	  {

//		data.print( F("+r,s") );
//	#if ESP8266
//		data.print( long( dir.fileSize()) );
//		data.print( F(",\t") );
//		data.println( dir.fileName() );
//	#else
//		data.print( long( fileDir.size()) );
//		data.print( F(",\t") );
//		data.println( fileDir.name() );
//	#endif



#ifdef ESP8266
	  String fn = dir.fileName();
	  long fz = long( dir.fileSize());
	  if (fn[0]=='/') { fn.remove(0, fn.lastIndexOf("/")+1); }
	  time_t time = dir.fileTime();
	  generateFileLine(&data, false, fn.c_str(), fz, time, this->user);
#else
	  long fz = long( fileDir.size());
	  const char* fnC = fileDir.name();
	  const char* fn;
	  if ( fnC[0] == '/' ) {
		  fn = &fnC[1];
	  }else{
		  fn = fnC;
	  }

	  time_t time = fileDir.getLastWrite();
	  generateFileLine(&data, false, fn, fz, time, this->user);

#endif

    nbMatch ++;
    return true;
  }
#elif STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SEEED_SD || STORAGE_TYPE == STORAGE_FFAT
	#if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
	  if( dir.next())
	#else
#if STORAGE_TYPE == STORAGE_SEEED_SD
	  FTP_FILE fileDir = STORAGE_MANAGER.open(dir.name());
	  fileDir = dir.openNextFile();
#else
	  FTP_FILE fileDir = dir.openNextFile();
#endif
	  if( fileDir )
#endif
	  {

	#if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
		  long fz = long( dir.fileSize());
//		  const char* fn = dir.fileName().c_str();
		  String aza = dir.fileName();
		  const char* fn = aza.c_str(); //Serial.printf("test %s ", fn);

//		data.print( long( dir.fileSize()) );
//		data.print( F(",\t") );
//		data.println( dir.fileName() );
	#elif STORAGE_TYPE == STORAGE_SEEED_SD
		  const char* fnC = fileDir.name();
		  const char* fn;
		  if ( fnC[0] == '/' ) {
			  fn = &fnC[1];
		  }else{
			  fn = fnC;
		  }
		long fz = fileDir.size();
	#else
		  long fz = long( fileDir.size());
		  const char* fn = fileDir.name();

//		data.print( long( fileDir.size()) );
//		data.print( F("\t") );
//		data.println( fileDir.name() );

//		DEBUG_PRINT( long( fileDir.size()));
//		DEBUG_PRINT( F("\t") );
//		DEBUG_PRINTLN( fileDir.name() );
	#endif
	#if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
		time_t time = dir.fileTime();
		generateFileLine(&data, dir.isDirectory(), fn, fz, time, this->user);
	#elif ESP32
		time_t time = fileDir.getLastWrite();
		generateFileLine(&data, fileDir.isDirectory(), fn, fz, time, this->user);
	#else
		generateFileLine(&data, fileDir.isDirectory(), fn, fz, "Jan 01 00:00", this->user);
	#endif
    nbMatch ++;
    return true;
  }
#elif STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC
	  FTP_FILE fileDir = dir.openNextFile();
	  if( fileDir )
	  {

//		data.print( F("+r,s") );
//		data.print( long( fileDir.size()) );
//		data.print( F(",\t") );
//		data.println( fileDir.name() );

		String fn = fileDir.name();
		if (fn[0]=='/') { fn.remove(0, fn.lastIndexOf("/")+1); }

		generateFileLine(&data, fileDir.isDirectory(), fn.c_str(), long( fileDir.size()), "Jan 01 00:00", this->user);

		nbMatch ++;
		return true;
  }

#elif STORAGE_TYPE == STORAGE_FATFS
  if( dir.nextFile())
  {
//    if( dir.isDir()) {
//      data.print( F("+/,\t") );
//    } else {
//    	data.print( F("+r,s") ); data.print( long( dir.fileSize()) ); data.print( F(",\t") );
//    }
//    data.println( dir.fileName() );

		String fn = dir.fileName();
		if (fn[0]=='/') { fn.remove(0, fn.lastIndexOf("/")+1); }

	generateFileLine(&data, dir.isDir(), fn.c_str(), long( dir.fileSize()), "Jan 01 00:00", this->user);

    nbMatch ++;
    return true;
  }
#else
  if( file.openNext( &dir, FTP_FILE_READ_ONLY ))
  {
//    if( file.isDir()) {
//      data.print( F("+/,\t") );
//    } else {
//    	data.print( F("+r,s") ); data.print( long( fileSize( file )) ); data.print( F(",\t") );
//    }

	generateFileLine(&data, file.isDir(), "", long( fileSize( file )), "Jan 01 00:00", this->user, false);

    file.printName( & data );
    data.println();
    file.close();
    nbMatch ++;
    return true;
  }
#endif
  client.print( F("226 ") ); client.print( nbMatch ); client.println( F(" matches total") );
#if STORAGE_TYPE != STORAGE_SPIFFS && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SEEED_SD
  dir.close();
#endif
  data.stop();
  return false;
}

bool FtpServer::doMlsd()
{
  if( ! dataConnected())
  {
#if STORAGE_TYPE != STORAGE_SPIFFS && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SEEED_SD
  dir.close();
#endif
  	DEBUG_PRINTLN(F("Not connected!!"));
    return false;
  }
 // DEBUG_PRINTLN(F("Connected!!"));

#if STORAGE_TYPE == STORAGE_SPIFFS
	  DEBUG_PRINTLN("DIR MLSD ");
	#if ESP8266
	  if( dir.next())
	#else
	  File fileDir = dir.openNextFile();
	  if( fileDir )
	#endif
	  {
		  DEBUG_PRINTLN("DIR NEXT ");
		char dtStr[ 15 ];

		struct tm * timeinfo;

		#if ESP8266
			time_t time = dir.fileTime();
		#else
			time_t time = fileDir.getLastWrite();
		#endif

			timeinfo = localtime ( &time );

			// 2000 01 01 16 06 56

			strftime (dtStr,15,"%Y%m%d%H%M%S",timeinfo);


	#if ESP8266
		String fn = dir.fileName();
		fn.remove(0, fn.lastIndexOf("/")+1);
		long fz = dir.fileSize();
	#else
		String fn = fileDir.name();
		fn.remove(0, fn.lastIndexOf("/")+1);
		long fz = fileDir.size();
	#endif

		data.print( F("Type=") );

		data.print( F("file") );
		data.print( F(";Modify=") ); data.print(dtStr);// data.print( makeDateTimeStr( dtStr, time, time) );
		data.print( F(";Size=") ); data.print( fz );
		data.print( F("; ") ); data.println( fn );

		// DEBUG_PRINT( F("Type=") );
		// DEBUG_PRINT( F("file") );

		// DEBUG_PRINT( F(";Modify=") ); DEBUG_PRINT(dtStr); //DEBUG_PRINT( makeDateTimeStr( dtStr, time, time) );
		// DEBUG_PRINT( F(";Size=") ); DEBUG_PRINT( fz );
		// DEBUG_PRINT( F("; ") ); DEBUG_PRINTLN( fn );

		DEBUG_PRINT( F("Type=file;Modify=%s;Size=%d;%s"),dtStr,fz,fn.c_str());

		nbMatch ++;
		return true;
	  }
#elif STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_SEEED_SD || STORAGE_TYPE == STORAGE_FFAT
	 // DEBUG_PRINTLN("DIR MLSD ");
	#if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
	  if( dir.next())
	#else
#if STORAGE_TYPE == STORAGE_SEEED_SD
	  File fileDir = STORAGE_MANAGER.open(dir.name());
	  fileDir = dir.openNextFile();
#else
	  File fileDir = dir.openNextFile();
#endif
	  // DEBUG_PRINTLN(dir);
	  // DEBUG_PRINTLN(fileDir);
	  if( fileDir )
	#endif
	  {
		 // DEBUG_PRINTLN("DIR NEXT ");
		char dtStr[ 15 ];


		#if STORAGE_TYPE == STORAGE_SEEED_SD
				struct tm * timeinfo;

				strcpy(dtStr, "19700101000000");
		#else
				struct tm * timeinfo;

				#if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
					time_t time = dir.fileTime();
				#else
					time_t time = fileDir.getLastWrite();
				#endif

					timeinfo = localtime ( &time );

					// 2000 01 01 16 06 56

					strftime (dtStr,15,"%Y%m%d%H%M%S",timeinfo);
		#endif

	#if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
		String fn = dir.fileName();
		long fz = dir.fileSize();
		FTP_DIR fileDir = dir;
	#elif STORAGE_TYPE == STORAGE_SEEED_SD
		String fn = fileDir.name();
		fn.remove(0, strlen(dir.name()));
		if (fn[0]=='/') { fn.remove(0, fn.lastIndexOf("/")+1); }
		long fz = fileDir.size();
	#else
		String fn = fileDir.name();
		fn.remove(0, fn.lastIndexOf("/")+1);
		long fz = fileDir.size();
	#endif

		data.print( F("Type=") );

		data.print( ( fileDir.isDirectory() ? F("dir") : F("file")) );
		data.print( F(";Modify=") ); data.print(dtStr);// data.print( makeDateTimeStr( dtStr, time, time) );
		data.print( F(";Size=") ); data.print( fz );
		data.print( F("; ") ); data.println( fn );

		DEBUG_PRINT( F("Type=%s;Modify=%s;Size=%d;%s"),fileDir.isDirectory() ? "dir" : "file",dtStr,fz,fn.c_str());

		nbMatch ++;
// RoSchmi: next line was commented
#if STORAGE_TYPE == STORAGE_SEEED_SD
		fileDir.close();
#endif
		return true;
	  }

#elif STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC
	  DEBUG_PRINTLN("DIR MLSD ");
	  File fileDir = dir.openNextFile();
	  if( fileDir )
	  {
		  DEBUG_PRINTLN("DIR NEXT ");
		char dtStr[ 15 ];

		struct tm * timeinfo;

		strcpy(dtStr, "19700101000000");


//		long fz = dir.fileSize();
		String fn = fileDir.name();

//#ifdef ESP32
		fn.remove(0, fn.lastIndexOf("/")+1);
//#else if !defined(ESP8266)
//		fn.remove(0, 1);
//#endif


		long fz = fileDir.size();

		data.print( F("Type=") );

		data.print( ( fileDir.isDirectory() ? F("dir") : F("file")) );
		data.print( F(";Modify=") ); data.print(dtStr);// data.print( makeDateTimeStr( dtStr, time, time) );
		data.print( F(";Size=") ); data.print( fz );
		data.print( F("; ") ); data.println( fn );

		DEBUG_PRINT( F("Type=") );
		DEBUG_PRINT( ( fileDir.isDirectory() ? F("dir") : F("file")) );

		DEBUG_PRINT( F(";Modify=") ); DEBUG_PRINT(dtStr); //DEBUG_PRINT( makeDateTimeStr( dtStr, time, time) );
		DEBUG_PRINT( F(";Size=") ); DEBUG_PRINT( fz );
		DEBUG_PRINT( F("; ") ); DEBUG_PRINTLN( fn );

		nbMatch ++;
		return true;
	  }

#elif STORAGE_TYPE == STORAGE_FATFS
  if( dir.nextFile())
  {
    char dtStr[ 15 ];
    data.print( F("Type=") ); data.print( ( dir.isDir() ? F("dir") : F("file")) );
    data.print( F(";Modify=") ); data.print( makeDateTimeStr( dtStr, dir.fileModDate(), dir.fileModTime()) );
    data.print( F(";Size=") ); data.print( long( dir.fileSize()) );
    data.print( F("; ") ); data.println( dir.fileName() );
    nbMatch ++;
    return true;
  }
#else
  if( file.openNext( &dir, FTP_FILE_READ_ONLY ))
  {
    char dtStr[ 15 ];
    uint16_t filelwd, filelwt;
    bool gfmt = getFileModTime( & filelwd, & filelwt );
    DEBUG_PRINT("gfmt --> ");
    DEBUG_PRINTLN(gfmt);
    if( gfmt )
    {
		  data.print( F("Type=") ); data.print( ( file.isDir() ? F("dir") : F("file")) );
		  data.print( F(";Modify=") ); data.print( makeDateTimeStr( dtStr, filelwd, filelwt ) );
		  data.print( F(";Size=") ); data.print( long( fileSize( file )) ); data.print( F("; ") );
		  file.printName( & data );
		  data.println();

		  DEBUG_PRINT( F("Type=") ); DEBUG_PRINT( ( file.isDir() ? F("dir") : F("file")) );
		  DEBUG_PRINT( F(";Modify=") ); DEBUG_PRINT( makeDateTimeStr( dtStr, filelwd, filelwt ) );
		  DEBUG_PRINT( F(";Size=") ); DEBUG_PRINT( long( fileSize( file )) ); DEBUG_PRINT( F("; ") );
//		  DEBUG_PRINT(file.name());
		  DEBUG_PRINTLN();
      nbMatch ++;
    }
    file.close();
    return gfmt;
  }
#endif
  client.println(F("226-options: -a -l") );
  client.print( F("226 ") ); client.print( nbMatch ); client.println( F(" matches total") );
#if STORAGE_TYPE != STORAGE_SPIFFS && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SEEED_SD && STORAGE_TYPE != STORAGE_SEEED_SD
    dir.close();
#endif
  data.stop();
  // DEBUG_PRINTLN(F("All file readed!!"));
  return false;
}

void FtpServer::closeTransfer()
{
  uint32_t deltaT = (int32_t) ( millis() - millisBeginTrans );
  if( deltaT > 0 && bytesTransfered > 0 )
  {
	  // DEBUG_PRINT( F(" Transfer completed in ") ); DEBUG_PRINT( deltaT ); DEBUG_PRINTLN( F(" ms, ") );
	  // DEBUG_PRINT( bytesTransfered / deltaT ); DEBUG_PRINTLN( F(" kbytes/s") );

	  // DEBUG_PRINT( F(" Transfer completed in %d ms, %d kbytes/s"), deltaT, bytesTransfered / deltaT );

	  if (FtpServer::_transferCallback) {
		  FtpServer::_transferCallback(FTP_TRANSFER_STOP, getFileName(&file), bytesTransfered);
	  }


    client.println(F("226-File successfully transferred") );
    client.print( F("226 ") ); client.print( deltaT ); client.print( F(" ms, ") );
    client.print( bytesTransfered / deltaT ); client.println( F(" kbytes/s") );
  }
  else
    client.println(F("226 File successfully transferred") );
  
  file.close();
  data.stop();
}

void FtpServer::abortTransfer()
{
  if( transferStage != FTP_Close )
  {
	  if (FtpServer::_transferCallback) {
		  FtpServer::_transferCallback(FTP_TRANSFER_ERROR, getFileName(&file), bytesTransfered);
	  }

	  file.close();
#if STORAGE_TYPE != STORAGE_SPIFFS && STORAGE_TYPE != STORAGE_LITTLEFS && STORAGE_TYPE != STORAGE_SEEED_SD
    dir.close();
#endif
    client.println(F("426 Transfer aborted") );
    DEBUG_PRINTLN( F(" Transfer aborted!") );

    transferStage = FTP_Close;
  }
//  if( data.connected())
  data.stop(); 
}

// Read a char from client connected to ftp server
//
//  update cmdLine and command buffers, iCL and parameter pointers
//
//  return:
//    -2 if buffer cmdLine is full
//    -1 if line not completed
//     0 if empty line received
//    length of cmdLine (positive) if no empty line received 

int8_t FtpServer::readChar()
{
  int8_t rc = -1;

  if( client.available())
  {
    char c = client.read();
    // DEBUG_PRINT("-");
    // DEBUG_PRINT( c );

    if( c == '\\' )
      c = '/';
    if( c != '\r' ){
      if( c != '\n' )
      {
        if( iCL < FTP_CMD_SIZE )
          cmdLine[ iCL ++ ] = c;
        else
          rc = -2; //  Line too long
      }
      else
      {
        cmdLine[ iCL ] = 0;
        command[ 0 ] = 0;
        parameter = NULL;
        // empty line?
        if( iCL == 0 )
          rc = 0;
        else
        {
          rc = iCL;
          // search for space between command and parameter
          parameter = strchr( cmdLine, ' ' );
          if( parameter != NULL )
          {
            if( parameter - cmdLine > 4 )
              rc = -2; // Syntax error
            else
            {
              strncpy( command, cmdLine, parameter - cmdLine );
              command[ parameter - cmdLine ] = 0;
              while( * ( ++ parameter ) == ' ' )
                ;
            }
          }
          else if( strlen( cmdLine ) > 4 )
            rc = -2; // Syntax error.
          else
            strcpy( command, cmdLine );
          iCL = 0;
        }
      }
    }
    if( rc > 0 )
      for( uint8_t i = 0 ; i < strlen( command ); i ++ )
        command[ i ] = toupper( command[ i ] );
    if( rc == -2 )
    {
      iCL = 0;
      client.println(F("500 Syntax error"));
    }
  }
  return rc;
}

bool FtpServer::haveParameter()
{
  if( parameter != NULL && strlen( parameter ) > 0 )
    return true;
  client.println("501 No file name" );
  return false;  
}

int utf8_strlen(const String& str)
{
    int c,i,ix,q;
    for (q=0, i=0, ix=str.length(); i < ix; i++, q++)
    {
        c = (unsigned char) str[i];
        if      (c>=0   && c<=127) i+=0;
        else if ((c & 0xE0) == 0xC0) i+=1;
        else if ((c & 0xF0) == 0xE0) i+=2;
        else if ((c & 0xF8) == 0xF0) i+=3;
        //else if (($c & 0xFC) == 0xF8) i+=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
        //else if (($c & 0xFE) == 0xFC) i+=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
        else return 0;//invalid utf8
    }
    return q;
}

//// ****** UTF8-Decoder: convert UTF8-string to extended ASCII *******
//static byte c1;  // Last character buffer
//
//// Convert a single Character from UTF8 to Extended ASCII
//// Return "0" if a byte has to be ignored
//byte utf8ascii(byte ascii) {
//    if ( ascii<128 )   // Standard ASCII-set 0..0x7F handling
//    {   c1=0;
//        return( ascii );
//    }
//
//    // get previous input
//    byte last = c1;   // get last char
//    c1=ascii;         // remember actual character
//
//    switch (last)     // conversion depending on first UTF8-character
//    {   case 0xC2: return  (ascii);  break;
//        case 0xC3: return  (ascii | 0xC0);  break;
//        case 0x82: if(ascii==0xAC) return(0x80);       // special case Euro-symbol
//    }
//
//    return  (0);                                     // otherwise: return zero, if character has to be ignored
//}
//
//// convert String object from UTF8 String to Extended ASCII
//String utf8ascii(String s)
//{
//        String r="";
//        char c;
//        for (int i=0; i<s.length(); i++)
//        {
//                c = utf8ascii(s.charAt(i));
//                if (c!=0) r+=c;
//        }
//        return r;
//}
//
//// In Place conversion UTF8-string to Extended ASCII (ASCII is shorter!)
//void utf8ascii(char* s)
//{
//        int k=0;
//        char c;
//        for (int i=0; i<strlen(s); i++)
//        {
//                c = utf8ascii(s[i]);
//                if (c!=0)
//                        s[k++]=c;
//        }
//        s[k]=0;
//}
//
//int utf8_strlen(const String& str)
//{
//	String ascii = utf8ascii(str);
//	return ascii.length();
//}
// Make complete path/name from cwdName and param
//
// 3 possible cases: param can be absolute path, relative path or only the name
//
// parameter:
//   fullName : where to store the path/name
//
// return:
//    true, if done

bool FtpServer::makePath( char * fullName, char * param )
{
  if( param == NULL )
    param = parameter;
    
  // Root or empty?
  if( strcmp( param, "/" ) == 0 || strlen( param ) == 0 )
  {
    strcpy( fullName, "/" );
    return true;
  }
  // If relative path, concatenate with current dir
  if( param[0] != '/' ) 
  {
    strcpy( fullName, cwdName );
    if( fullName[ strlen( fullName ) - 1 ] != '/' )
      strncat( fullName, "/", FTP_CWD_SIZE );
    strncat( fullName, param, FTP_CWD_SIZE );
  }
  else
    strcpy( fullName, param );
  // If ends with '/', remove it
  uint16_t strl = strlen( fullName ) - 1;
  if( fullName[ strl ] == '/' && strl > 1 )
    fullName[ strl ] = 0;
  if( strlen( fullName ) >= FTP_CWD_SIZE )
  {
    client.println(F("500 Command line too long"));
    return false;
  }
#ifdef UTF8_SUPPORT
//  for( uint8_t i = 0; i < utf8_strlen( fullName ); i ++ ) {
//
//  }


//  DEBUG_PRINT(F("utf8_strlen2"));
//  DEBUG_PRINTLN(utf8_strlen2(fullName));

  if (utf8_strlen(fullName)>=FILENAME_LENGTH) {
      DEBUG_PRINT(F("utf8_strlen %d"),utf8_strlen(fullName));
      client.println(F("553 File name not allowed. Too long.") );
      return false;
  }
#else
  for( uint8_t i = 0; i < strlen( fullName ); i ++ ) {
    if( ! legalChar( fullName[i]))
    {
      client.println(F("553 File name not allowed") );
      return false;
    }
  }
  if (strlen(fullName)>=FILENAME_LENGTH) {
      client.println(F("553 File name not allowed. Too long.") );
      return false;
  }
#endif
  return true;
}

bool FtpServer::makeExistsPath( char * path, char * param )
{
  if( ! makePath( path, param ))
    return false;
  // RoSchmi
  //#if STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_SD
#if (STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC  || STORAGE_TYPE == STORAGE_SEEED_SD)
  if (strcmp(path, "/") == 0)  return true;
#endif
  DEBUG_PRINT("PATH %s %s",path,exists( path )?"found":"not found");
  // DEBUG_PRINT(path)
  if( exists( path )) {
	  // DEBUG_PRINTLN(" ...EXIST!")
    return true;
  }
  // DEBUG_PRINTLN(" ...NOT EXIST!")
  client.print(F("550 ")); client.print( path ); client.println( F(" not found.") );
  return false;
}

// Calculate year, month, day, hour, minute and second
//   from first parameter sent by MDTM command (YYYYMMDDHHMMSS)
// Accept longer parameter YYYYMMDDHHMMSSmmm where mmm are milliseconds
//   but don't take in account additional digits
//
// parameters:
//   dt: 15 length string for 14 digits and terminator
//   pyear, pmonth, pday, phour, pminute and psecond: pointer of
//     variables where to store data
//
// return:
//    0 if parameter is not YYYYMMDDHHMMSS
//    length of parameter + space
//
// Date/time are expressed as a 14 digits long string
//   terminated by a space and followed by name of file

uint8_t FtpServer::getDateTime( char * dt, uint16_t * pyear, uint8_t * pmonth, uint8_t * pday,
                                uint8_t * phour, uint8_t * pminute, uint8_t * psecond )
{
  uint8_t i;
  dt[ 0 ] = 0;
  if( strlen( parameter ) < 15 ) //|| parameter[ 14 ] != ' ' )
    return 0;
  for( i = 0; i < 14; i ++ )
    if( ! isdigit( parameter[ i ]))
      return 0;
  for( i = 14; i < 18; i ++ )
    if( parameter[ i ] == ' ' )
      break;
    else if( ! isdigit( parameter[ i ]))
      return 0;
  if( i == 18 )
    return 0;
  i ++ ;
  
  strncpy( dt, parameter, 14 );
  dt[ 14 ] = 0;
  * psecond = atoi( dt + 12 ); 
  dt[ 12 ] = 0;
  * pminute = atoi( dt + 10 );
  dt[ 10 ] = 0;
  * phour = atoi( dt + 8 );
  dt[ 8 ] = 0;
  * pday = atoi( dt + 6 );
  dt[ 6 ] = 0 ;
  * pmonth = atoi( dt + 4 );
  dt[ 4 ] = 0 ;
  * pyear = atoi( dt );
  strncpy( dt, parameter, 14 );
  // DEBUG_PRINT( F(" Modification time: ") ); DEBUG_PRINT( * pyear ); DEBUG_PRINT( F("/") ); DEBUG_PRINT( int(* pmonth) ); DEBUG_PRINT( F("/") ); DEBUG_PRINT( int(* pday) );
  // DEBUG_PRINT( F(" ") ); DEBUG_PRINT( int(* phour) ); DEBUG_PRINT( F(":") ); DEBUG_PRINT( int(* pminute) ); DEBUG_PRINT( F(":") ); DEBUG_PRINT( int(* psecond) );
  // DEBUG_PRINT( F(" of file: ") ); DEBUG_PRINTLN( (char *) ( parameter + i ) );

  return i;
}

// Create string YYYYMMDDHHMMSS from date and time
//
// parameters:
//    date, time 
//    tstr: where to store the string. Must be at least 15 characters long
//
// return:
//    pointer to tstr

char * FtpServer::makeDateTimeStr( char * tstr, uint16_t date, uint16_t time )
{
  sprintf( tstr, "%04u%02u%02u%02u%02u%02u",
           (( date & 0xFE00 ) >> 9 ) + 1980, ( date & 0x01E0 ) >> 5, date & 0x001F,
           ( time & 0xF800 ) >> 11, ( time & 0x07E0 ) >> 5, ( time & 0x001F ) << 1 );            
  return tstr;
}


uint32_t FtpServer::fileSize( FTP_FILE file ) {
#if (STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_FFAT || STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC || STORAGE_TYPE == STORAGE_SEEED_SD)
	return file.size();
#else
	return file.fileSize();
#endif
}

#if (STORAGE_TYPE == STORAGE_SEEED_SD)
  bool FtpServer::openFile( char path[ FTP_CWD_SIZE ], int readTypeInt ){
		DEBUG_PRINT(F("File to open ") );
		DEBUG_PRINT( path );
		DEBUG_PRINT(F(" readType ") );
		DEBUG_PRINTLN(readTypeInt);

		if (readTypeInt == 0X01) {
			readTypeInt = FILE_READ;
		}else {
			readTypeInt = FILE_WRITE;
		}

		file = STORAGE_MANAGER.open( path, readTypeInt );
		if (!file) { // && readTypeInt[0]==FILE_READ) {
			return false;
		}else{
			// DEBUG_PRINTLN("TRUE");

			return true;
		}
}
#elif ((STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC) && defined(ESP8266))// FTP_SERVER_NETWORK_TYPE_SELECTED == NETWORK_ESP8266_242)
  bool FtpServer::openFile( char path[ FTP_CWD_SIZE ], int readTypeInt ){
		DEBUG_PRINT(F("File to open ") );
		DEBUG_PRINT( path );
		DEBUG_PRINT(F(" readType ") );
		DEBUG_PRINTLN(readTypeInt);

		if (readTypeInt == 0X01) {
			readTypeInt = FILE_READ;
		}else {
			readTypeInt = FILE_WRITE;
		}

		file = STORAGE_MANAGER.open( path, readTypeInt );
		if (!file) { // && readTypeInt[0]==FILE_READ) {
			return false;
		}else{
			DEBUG_PRINTLN("TRUE");

			return true;
		}
}
#elif (STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_FFAT )
  bool FtpServer::openFile( const char * path, const char * readType ) {
  		DEBUG_PRINT(F("File to open %s"), path );
  		// DEBUG_PRINT( path );
  		// DEBUG_PRINT(F(" readType ") );
  		// DEBUG_PRINTLN(readType);
  		file = STORAGE_MANAGER.open( path, readType );
  		if (!file && readType[0]=='r') {
  			return false;
  		}else{
  			// DEBUG_PRINTLN("TRUE");

  			return true;
  		}
  }
#elif STORAGE_TYPE <= STORAGE_SDFAT2 || STORAGE_TYPE == STORAGE_SPIFM || ((STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC) && ARDUINO_ARCH_SAMD)
  bool FtpServer::openFile( char path[ FTP_CWD_SIZE ], int readTypeInt ){
		DEBUG_PRINT(F("File to open ") );
		DEBUG_PRINT( path );
		DEBUG_PRINT(F(" readType ") );
		DEBUG_PRINTLN(readTypeInt);

		file = STORAGE_MANAGER.open( path, readTypeInt );
		if (!file) {
			return false;
		}else{
			DEBUG_PRINTLN("TRUE");

			return true;
		}
}

#else
  bool FtpServer::openFile( char path[ FTP_CWD_SIZE ], const char * readType ) {
  	return openFile( (const char*) path, readType );
  }
  bool FtpServer::openFile( const char * path, const char * readType ) {
  		DEBUG_PRINT(F("File to open ") );
  		DEBUG_PRINT( path );
  		DEBUG_PRINT(F(" readType ") );
  		DEBUG_PRINTLN(readType);
  #if ((STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC) && !defined(ESP32))
  		if (readType == 0X01) {
  			readType = FILE_READ;
  		}else {
  			readType = FILE_WRITE;
  		}
  #endif
  		file = STORAGE_MANAGER.open( path, readType );
  		if (!file && readType[0]=='r') {
  			return false;
  		}else{
  			DEBUG_PRINTLN("TRUE");

  			return true;
  		}
  }
#endif

// Return true if path points to a directory
bool FtpServer::isDir( char * path )
{
#if (STORAGE_TYPE == STORAGE_LITTLEFS && (defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)))
	  FTP_DIR dir;
	  bool res;
	  dir = STORAGE_MANAGER.openDir( path );

	  res = true;
  	  return res;

	  #elif STORAGE_TYPE == STORAGE_SPIFFS
	if (strcmp(path, "/") == 0)  { return true; }
	return false; // no directory support
#elif STORAGE_TYPE == STORAGE_SEEED_SD || STORAGE_TYPE == STORAGE_FFAT || (STORAGE_TYPE == STORAGE_LITTLEFS && defined(ESP32))
	  FTP_DIR dir;
	  bool res;
	  dir = STORAGE_MANAGER.open( path );

//	  return true;
	  res = dir.isDirectory();
	  return res;
#elif STORAGE_TYPE == STORAGE_FATFS
  return STORAGE_MANAGER.isDir( path );
#elif STORAGE_TYPE == STORAGE_SDFAT1 || STORAGE_TYPE == STORAGE_SDFAT2
//  bool res = (!dir.open(path, FTP_FILE_READ) || !dir.isDir());
//  dir.close();
//  return res;
  if (strcmp(path, "/") == 0)  { return true; }
  if( ! openFile( path, FTP_FILE_READ )) {
      return false;
    }
  return true;
#else
  FTP_FILE file;
  bool res;
  
  if( ! openFile( path, FTP_FILE_READ )) {
    return false;
  }
#if STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC
//  if (strcmp(path, "/") == 0) return true;
//  res = file.isDirectory();
//  DEBUG_PRINT(path);
//  DEBUG_PRINT(" IS DIRECOTORY --> ");
//  DEBUG_PRINTLN(res);
  return true;
#else
//  res = file.isDir();
//  DEBUG_PRINT("IS DIRECTORY --> " );
//  DEBUG_PRINTLN(res);
#endif
  file.close();
  return res;
#endif
}

bool FtpServer::timeStamp( char * path, uint16_t year, uint8_t month, uint8_t day,
                           uint8_t hour, uint8_t minute, uint8_t second )
{
#if STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS  || STORAGE_TYPE == STORAGE_FFAT || STORAGE_TYPE == STORAGE_SD || STORAGE_TYPE == STORAGE_SD_MMC || STORAGE_TYPE == STORAGE_SEEED_SD
//	struct tm tmDate = { second, minute, hour, day, month, year };
//    time_t rawtime = mktime(&tmDate);

    return true;
	// setTime(rawtime);
	// SPIFFS USE time() call
//  return STORAGE_MANAGER.timeStamp( path, year, month, day, hour, minute, second );
#elif STORAGE_TYPE == STORAGE_FATFS
  return STORAGE_MANAGER.timeStamp( path, year, month, day, hour, minute, second );
#else
  FTP_FILE file;
  bool res;

  if( ! openFile( path, FTP_FILE_READ_WRITE ))
    return false;
  res = file.timestamp( T_WRITE, year, month, day, hour, minute, second );
  file.close();
  return res;
#endif
}
                        
bool FtpServer::getFileModTime( char * path, uint16_t * pdate, uint16_t * ptime )
{
#if STORAGE_TYPE == STORAGE_FATFS
  return STORAGE_MANAGER.getFileModTime( path, pdate, ptime );
#else
//  FTP_FILE file;
  bool res;

  if( ! openFile( path, FTP_FILE_READ )) {
    return false;
  }
  res = getFileModTime( pdate, ptime );
  file.close();
  return res;
#endif
}

// Assume SD library is SdFat (or family) and file is open
                        
#if STORAGE_TYPE != STORAGE_FATFS
bool FtpServer::getFileModTime( uint16_t * pdate, uint16_t * ptime )
{
#if STORAGE_TYPE == STORAGE_SPIFFS || STORAGE_TYPE == STORAGE_LITTLEFS || STORAGE_TYPE == STORAGE_FFAT
	#if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
		return dir.fileTime();
	#else
		return dir.getLastWrite();
	#endif
#elif STORAGE_TYPE == STORAGE_SDFAT1
  dir_t d;

  if( ! file.dirEntry( & d ))
    return false;
  * pdate = d.lastWriteDate;
  * ptime = d.lastWriteTime;
  return true;
#elif  STORAGE_TYPE == STORAGE_SDFAT2  || STORAGE_TYPE == STORAGE_SPIFM
  return file.getModifyDateTime( pdate, ptime );
#endif
  return false;
}
#endif

#if STORAGE_TYPE == STORAGE_SD ||  STORAGE_TYPE == STORAGE_SD_MMC
  bool     FtpServer::rename( const char * path, const char * newpath ){

		FTP_FILE myFileIn = STORAGE_MANAGER.open(path, FILE_READ);
		FTP_FILE myFileOut = STORAGE_MANAGER.open(newpath, FILE_WRITE);

		if(myFileOut) {
			while (myFileIn.available() > 0)
			      {
			        int i = myFileIn.readBytes((char*)buf, FTP_BUF_SIZE);
			        myFileOut.write(buf, i);
			      }
			      // done, close the destination file
				myFileOut.close();
				myFileOut = STORAGE_MANAGER.open(newpath, FILE_READ);

		}
		bool operation = false;

		DEBUG_PRINT(F("RENAME --> "));
		DEBUG_PRINT(myFileIn.size());
		DEBUG_PRINT(" size ");
		DEBUG_PRINTLN(myFileOut.size());

		if (myFileIn.size() == myFileOut.size()) {
			operation = true;
		}


		if (!operation) return operation;

		myFileIn.close();
		myFileOut.close();

		return remove( path );
  };
#endif
