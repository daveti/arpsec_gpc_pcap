#ifndef AsLogic_INCLUDED
#define AsLogic_INCLUDED

////////////////////////////////////////////////////////////////////////////////
//
//  File          : AsLogic.h
//  Description   : The AsLogic module implements a shim for the logic
//                  program for the arpsec daemnon.
//
//  Description(new): The AsLogic module is rewritten using the GProlog/C interfaces
//
//  Author : Dave Tian
//  Rewritten : Mon Dec  2 13:13:05 PST 2013
//  Author : Patrick McDaniel
//  Created : Sun Mar 24 06:53:30 EDT 2013
//  Dev    : Dave Tian
//  Modified: Sat Nov 16 10:25:58 PST 2013

// Project Includes
#include "AsDefs.h"

//
// Module methods

// Disable the logic layer for arpsecd
void aslDisableLogic( void );
	
// Initialize the interface to the Logic engine (with parameters)
int aslInitLogic( void );
	
// Shutdown the PL interface
int aslShutdownLogic( void );

//
// Assertion of trust statements
	
// Add a trust statement to the logic
int aslAddTrustStatement( AsSystem s, AsTime t );

// Add a binding statement to the logic
int aslAddBindingStatement( AsSystem s, AsMediaAddress m, AsNetworkAddress n, AsTime t );
	
//
// Logic query methods
	
// Find the valid binding for network address N at time T
int aslFindValidMediaBinding( AsNetworkAddress n, AsMediaAddress m, AsTime t );

// Find the valid binding for media address M at time T
int aslFindValidNetworkBinding( AsNetworkAddress n, AsMediaAddress m, AsTime t );

// Determine if system S is trusted at time T
int aslSystemTrusted( AsSystem s, AsTime t );


//
// Unit testing

// Validate logic interface with randomized inputs
int testAsLogicInterfaces( void );


#endif
