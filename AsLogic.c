////////////////////////////////////////////////////////////////////////////////
//
//  File	  : AsLogic.c
//  Description   : The AsLogic module implements a shim for the logic
//		    program for the arpsec daemnon.
//
//  Description(New): The AsLogic module is taking the advantage of GProlog C
//		      interfaces instead of doing interactive IPC with GProlog
//		      process itself.
//
//  Author  : Dave Tian
//  Modified: Tue Nov 26 10:16:40 PST 2013
//  
//  Author  : Patrick McDaniel
//  Created : Sun Mar 24 07:21:50 EDT 2013
//
//  Modified: Jul 7, 2013
//  By	    : daveti
//
//

// Includes
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <gprolog.h>
// Project Includes
#include "AsLogic.h"
#include "AsLog.h"

// daveti: Defs for gpc calls in the GProlog
#define GPC_FUNC_TRUSTED			"trusted"
#define GPC_FUNC_VALID_BINDING			"valid_binding"
#define GPC_FUNC_VALID_STATEMENT		"valid_statement"
#define GPC_FUNC_ASSERTA			"asserta"
#define GPC_FUNC_DYNAMIC_TRUST_STATEMENT	"trust_statement"
#define GPC_FUNC_DYNAMIC_BINDING_STATEMENT	"binding_statement"
#define GPC_MAX_SOLUTION_NUM			1000

// daveti: Global gpc func numbers
// Needs to be init'd during the gpc init
static int gpc_func_trusted_num;
static int gpc_func_valid_binding_num;
static int gpc_func_valid_statement_num;
static int gpc_func_asserta_num;
static int gpc_func_dynamic_trust_statement_num;
static int gpc_func_dynamic_binding_statement_num;

//
// Module local data
int	aslogic_initialized = 0;    // Flag indicating that the library has been initialized
int	aslDisableLogicFlag = 0;    // daveti: for better performance, logic layer could be disabled!

////////////////////////////////////////////////////////////////////////////////
//
// Function     : aslDisableLogic
// Description  : Disable the logic layer even though it is init'd
//
// Inputs       : void
// Outputs      : void

void aslDisableLogic( void )
{
	aslDisableLogicFlag = 1;
	asLogMessage("arpsecd: Warning - Logic layer (gprolog) is disabled");
}

////////////////////////////////////////////////////////////////////////////////
//
// Function	: aslInitLogic
// Description	: Initialize the interface to the Logic engine (with parameters)
//
// Inputs	: argc - the number of parameters
//		  argv - points to the parameters
// Outputs	: 0 if successful, -1 if not

int aslInitLogic( void ) {

    // Check if already initialized
    if ( aslogic_initialized ) return( 0 );
	
    // Initialize and set internal values
    // NOTE: gprolog needs ration argc/argv or it crashes, so I am faking it
    asLogMessage( "Intializing arpsecd logic ..." );
    // fake parameters for gpc - but never used anyway
    int argc;
    char *argv[1];
    Pl_Start_Prolog(argc, argv);
    // Init the gpc func numbers here
    gpc_func_trusted_num = Pl_Find_Atom(GPC_FUNC_TRUSTED);
    gpc_func_valid_binding_num = Pl_Find_Atom(GPC_FUNC_VALID_BINDING);
    gpc_func_valid_statement_num = Pl_Find_Atom(GPC_FUNC_VALID_STATEMENT);
    gpc_func_asserta_num = Pl_Find_Atom(GPC_FUNC_ASSERTA);
    gpc_func_dynamic_trust_statement_num = Pl_Find_Atom(GPC_FUNC_DYNAMIC_TRUST_STATEMENT);
    gpc_func_dynamic_binding_statement_num = Pl_Find_Atom(GPC_FUNC_DYNAMIC_BINDING_STATEMENT);
    // Mark it init'd
    aslogic_initialized = 1;
    asLogMessage( "arpsecd logic (gpc) initialized." );
    asLogMessage("GPC DEBUG:\n"
		"func(%s) = [%d]\n"
		"func(%s) = [%d]\n"
		"func(%s) = [%d]\n"
		"func(%s) = [%d]\n"
		"func(%s) = [%d]\n"
		"func(%s) = [%d]",
		GPC_FUNC_TRUSTED, gpc_func_trusted_num,
		GPC_FUNC_VALID_BINDING, gpc_func_valid_binding_num,
		GPC_FUNC_VALID_STATEMENT, gpc_func_valid_statement_num,
		GPC_FUNC_ASSERTA, gpc_func_asserta_num,
		GPC_FUNC_DYNAMIC_TRUST_STATEMENT, gpc_func_dynamic_trust_statement_num,
		GPC_FUNC_DYNAMIC_BINDING_STATEMENT, gpc_func_dynamic_binding_statement_num
		);

    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function	: aslShutdownLogic
// Description	: Initialize the interface to the Logic engine (with parameters)
//
// Inputs	: argc - the number of parameters
//		  argv - points to the parameters
// Outputs	: 0 if successful, -1 if not

// Shutdown the PL interface
int aslShutdownLogic( void ) {

	// Check if not initialized, close interface
	if ( ! aslogic_initialized ) return( 0 );
	asLogMessage( "Stopping arpsecd logic ..." );
	// Instead, let's shutdown the gpc query context
	Pl_Stop_Prolog();
	asLogMessage( "Done." );
	aslogic_initialized = 0;
	return( 0 );
}

//
// Assertion of trust statements

////////////////////////////////////////////////////////////////////////////////
//
// Function	: aslAddTrustStatement
// Description	: Add a trust statement to the logic
//
// Inputs	: s - system that was trusted
//		  t - time at which the trust statement was made
// Outputs	: 0 if successful, -1 if not
// Reference	: snprintf( cmd, 256, "asserta(trust_statement(%s,%lu)).\n", s, t );

int aslAddTrustStatement( AsSystem s, AsTime t ) {

    // Check if the logic layer is disabled
    if (aslDisableLogicFlag == 1)
	return 0;

    // Local variables
    PlTerm arg[10];
    PlTerm arg2[10];
    PlBool res;

    // Start processing, if needed 
    if ( ! aslogic_initialized ) aslInitLogic();

    // Do the GPC query
    Pl_Query_Begin(PL_TRUE);
    arg[0] = Pl_Mk_String(s);
    arg[1] = Pl_Mk_Integer(t);
    arg2[0] = Pl_Mk_Callable(gpc_func_dynamic_trust_statement_num, 2, arg);
    res = Pl_Query_Call(gpc_func_asserta_num, 1, arg2);
    Pl_Query_End(PL_RECOVER);

    // Get output and process
    if (res != 1) {
	asLogMessage( "Prolog launch failed, aborting." );
	exit( -1 );
    }
    asLogMessage( "Trust statement added successfully (%s,%lu)", s, t );

    // Return successfully
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function	: AddBindingStatement
// Description	: Add a binding statement to the logic
//
// Inputs	: s - system that makes the binding assertion
//		  m - the media address
//		  n - the network address
//		  t - time at which the statement was made
// Outputs	: 0 if successful, -1 if not
// Reference	: snprintf( cmd, 256, "asserta(binding_statement(%s,%s,%s,%lu)).\n", s, n, m, t );

int aslAddBindingStatement( AsSystem s, AsMediaAddress m, AsNetworkAddress n, AsTime t ) {

    // Check if the logic layer is disabled
    if (aslDisableLogicFlag == 1)
	return 0;

    // Local variables
    PlTerm arg[10];
    PlTerm arg2[10];
    PlBool res;

    // Start processing, if needed 
    if ( ! aslogic_initialized ) aslInitLogic();

    // Do the GPC query
    Pl_Query_Begin(PL_TRUE);
    arg[0] = Pl_Mk_String(s);
    arg[1] = Pl_Mk_String(n);
    arg[2] = Pl_Mk_String(m);
    arg[3] = Pl_Mk_Integer(t);
    arg2[0] = Pl_Mk_Callable(gpc_func_dynamic_binding_statement_num, 4, arg);
    res = Pl_Query_Call(gpc_func_asserta_num, 1, arg2);
    Pl_Query_End(PL_RECOVER);

    // Get output and process
    if (res != 1) {
	asLogMessage( "Prolog launch failed, aborting." );
	exit( -1 );
    }
    asLogMessage( "Binding statement added successfully (%s,%lu)", s, t );

    // Return successfully
    return( 0 );
}
	
//
// Logic query methods

////////////////////////////////////////////////////////////////////////////////
//
// Function	: FindValidMediaBinding
// Description	: Find the valid binding for network address N at time T
//
// Inputs	: n - the network address to find binding fore
//		  m - the media address structure to copy into 
//		  t - time at which the binding should be true
// Outputs	: 1 if found, 0 if not
// Reference	: snprintf( cmd, 256, "valid_binding(%s,X,%lu).\n", n, t );
	
int aslFindValidMediaBinding( AsNetworkAddress n, AsMediaAddress m, AsTime t ) {

    // Check if the logic layer is disabled
    if (aslDisableLogicFlag == 1)
	return 0;

    // Local variables
    int i;
    int sol_num = 0;
    PlTerm arg[10];
    char *sol[GPC_MAX_SOLUTION_NUM];
    PlBool res;

    // Start processing, if needed 
    if ( ! aslogic_initialized ) aslInitLogic();

    // Do the GPC query
    Pl_Query_Begin(PL_TRUE);
    arg[0] = Pl_Mk_String(n);
    arg[1] = Pl_Mk_Variable();
    arg[2] = Pl_Mk_Integer(t);
    res = Pl_Query_Call(gpc_func_valid_binding_num, 3, arg);
    while (res)
    {
	sol[sol_num++] = Pl_Rd_String(arg[1]);
	res = Pl_Query_Next_Solution();
	if (sol_num >= GPC_MAX_SOLUTION_NUM)
	{
		asLogMessage("Error: aslFindValidMediaBinding got more than [%d] solutions - searching stopped",
				GPC_MAX_SOLUTION_NUM);
		break;
	}
    }
    Pl_Query_End(PL_RECOVER);

    // Parse out the solutions
    asLogMessage("Info: aslFindValidMediaBinding got [%d] solutions in total", sol_num);
    for (i = 0; i < sol_num; i++)
    {
	// Copy and remove trailing space
	strncpy( m, sol[i], MAX_MEDADDR_LENGTH );
	asLogMessage( "Find valid media binding successfully (%s,%lu)->[%s]", n, t, m );

	// Return we found it - currently is the first solution we have found
	return( 1 );
    }

    // Return successfully
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function	: FindValidNetworkBinding
// Description	: Find the valid binding for network address N at time T
//
// Inputs	: n - the network address structure to place the value in
//		  m - the network address to find binding fore
//		  t - time at which the binding should be true
// Outputs	: 1 if found, 0 if not
// Reference	: snprintf( cmd, 256, "valid_binding(X,%s,%lu).\n", m, t );

int aslFindValidNetworkBinding( AsNetworkAddress n, AsMediaAddress m, AsTime t )  {

    // Check if the logic layer is disabled
    if (aslDisableLogicFlag == 1)
        return 0;

    // Local variables
    int i;
    int sol_num = 0;
    PlTerm arg[10];
    char *sol[GPC_MAX_SOLUTION_NUM];
    PlBool res;

    // Start processing, if needed 
    if ( ! aslogic_initialized ) aslInitLogic();

    // Do the GPC query
    Pl_Query_Begin(PL_TRUE);
    arg[0] = Pl_Mk_Variable();
    arg[1] = Pl_Mk_String(m);
    arg[2] = Pl_Mk_Integer(t);
    res = Pl_Query_Call(gpc_func_valid_binding_num, 3, arg);
    while (res)
    {
	sol[sol_num++] = Pl_Rd_String(arg[0]);
	res = Pl_Query_Next_Solution();
	if (sol_num >= GPC_MAX_SOLUTION_NUM)
	{
		asLogMessage("Error: aslFindValidNetworkBinding got more than [%d] solutions - searching stopped",
				GPC_MAX_SOLUTION_NUM);
		break;
	}
    }
    Pl_Query_End(PL_RECOVER);

    // Parse out the solutions
    asLogMessage("Info: aslFindValidNetworkBinding got [%d] solutions in total", sol_num);
    for (i = 0; i < sol_num; i++)
    {
	// Copy and remove trailing space
	strncpy( n, sol[i], MAX_NETADDR_LENGTH );
	asLogMessage( "Find valid network binding successfully (%s,%lu)->[%s]", m, t, n );

	// Return we found it - currently is the first solution we have found
	return( 1 );
    }

    // Return successfully
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function	: aslSystemTrusted
// Description	: Determine if system S is trusted at time T
//
// Inputs	: s - system to be trusted
//		  t - time at which the system is trusted
// Outputs	: 1 if trusted, 0 if not
// Reference	: snprintf( cmd, 256, "trusted(%s,%lu).\n", s, t );

int aslSystemTrusted( AsSystem s, AsTime t ) {

    // Check if the logic layer is disabled
    if (aslDisableLogicFlag == 1)
	return 0;

    // Local variables
    PlTerm arg[10];
    PlBool res;

    // Start processing, if needed 
    if ( ! aslogic_initialized ) aslInitLogic();

    // Do the GPC query
    Pl_Query_Begin(PL_TRUE);
    arg[0] = Pl_Mk_String(s);
    arg[1] = Pl_Mk_Integer(t);
    res = Pl_Query_Call(gpc_func_trusted_num, 2, arg);
    Pl_Query_End(PL_RECOVER);

    // Just reture the res!
    return( res );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function	: testAsLogicInterface
// Description	: Validate logic interface with randomized inputs
//
// Inputs	: none
// Outputs	: 0 if successful, -1 if not

int testAsLogicInterfaces( void ) {

    // Set randomized information
    int rnd = as_random(100), rnd2 = as_random(100);
    char sys[32], media[MAX_MEDADDR_LENGTH], network[MAX_NETADDR_LENGTH];
    sprintf( sys, "sys%03d", rnd );
    AsNetworkAddress n = network;
    AsMediaAddress m = media;

    // Now call the logic functions
    asLogMessage( "Unit testing the arpsec logic with randomized data." );
    asLogMessage( "Adding test trust statment for system [%s], time [%d].", sys, rnd2 );
    aslAddTrustStatement( sys, rnd2 );
    aslSystemTrusted( sys, rnd2 );
    aslAddBindingStatement( sys, "mediaA", "netB", rnd2 ); 
    aslFindValidMediaBinding( "netB", m, rnd2+1 );
    aslFindValidNetworkBinding( n, "mediaA", rnd2+1 );
    asLogMessage( "Unit testing complete." );

    // Return sucessfully
    return( 0 );
}

