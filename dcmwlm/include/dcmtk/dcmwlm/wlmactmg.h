/*
 *
 *  Copyright (C) 1996-2022, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module:  dcmwlm
 *
 *  Author:  Thomas Wilkens
 *
 *  Purpose: Activity manager class for basic worklist management service
 *           class provider engines.
 *
 */

#ifndef WlmActivityManager_h
#define WlmActivityManager_h

#include "dcmtk/config/osconfig.h"
#include "dcmtk/ofstd/ofcmdln.h"   /* for OFCmdUnsignedInt */
#include "dcmtk/dcmdata/dcxfer.h"  /* for E_TransferSyntax */
#include "dcmtk/dcmnet/dimse.h"    /* for T_DIMSE_BlockingMode */
#include "dcmtk/dcmwlm/wltypdef.h" /* for WlmRefuseReasonType */

class WlmDataSource;
class OFCondition;

/** This class encapsulates data structures and operations for basic worklist management service
 *  class providers.
 */
class DCMTK_DCMWLM_EXPORT WlmActivityManager
{
  protected:
    /// data source connection object
    WlmDataSource *dataSource;
    /// port on which the application is listening
    OFCmdUnsignedInt opt_port;
    /// indicates if the application shall refuse any association
    OFBool opt_refuseAssociation;
    /// indicates if the application shall reject associations without implementation class UIDs
    OFBool opt_rejectWithoutImplementationUID;
    /// indicates how many seconds the application is supposed to sleep before handling a find request
    OFCmdUnsignedInt opt_sleepBeforeFindReq;
    /// indicates how long the application shall sleep after a find
    OFCmdUnsignedInt opt_sleepAfterFind;
    /// indicates how long the application shall sleep during a find
    OFCmdUnsignedInt opt_sleepDuringFind;
    /// max PDU size
    OFCmdUnsignedInt opt_maxPDU;
    /// preferred network transfer syntax
    E_TransferSyntax opt_networkTransferSyntax;
    /// indicates if the application shall fail on an invalid C-Find RQ message
    OFBool opt_failInvalidQuery;
    /// indicates if the application is run in single process mode or not
    OFBool opt_singleProcess;
    /// indicates, that this process was spawn as child from a parent process
    /// needed for multiprocess mode on WIN32
    OFBool opt_forkedChild;
    /// number of arguments in commandline, needed for multiprocess mode on WIN32
    int cmd_argc;
    /// complete command line, needed for multiprocess mode on WIN32
		const char **cmd_argv;
    /// maximum number of association for non-single process mode
    int opt_maxAssociations;
    /// blocking mode for DIMSE operations
    T_DIMSE_BlockingMode opt_blockMode;
    /// timeout for DIMSE operations
    int opt_dimse_timeout;
    /// timeout for ACSE operations
    int opt_acse_timeout;
    /// array of supported abstract syntaxes
    char **supportedAbstractSyntaxes;
    /// number of array fields
    int numberOfSupportedAbstractSyntaxes;
    /// table of processes for non-single process mode
    OFList<WlmProcessSlotType*> processTable;
    /// the directory where to store request files to
    OFString opt_requestFilePath;
    /// the format used for the request file names.
    /// Several placeholders can be used by(denoted by #) :
    ///   \#a: calling application entity title of the peer SCU
    ///   \#c: called application entity title (AE title of worklist SCP application)
    ///   \#i process id of the worklist SCP application process handling the request
    ///   \#p: patient ID if present, otherwise empty string
    ///   \ #t: timestamp in the format YYYYMMDDhhmmssffffff
    OFString opt_requestFileFormat;

      /** This function takes care of receiving, negotiating and accepting/refusing an
       *  association request. Additionally, it handles the request the association
       *  requesting application transmits after a connection is established.
       *  @param net Contains network parameters.
       */
    OFCondition WaitForAssociation( T_ASC_Network *net );

      /** This function takes care of removing items referring to (terminated) subprocess
       *  from the table which stores all subprocess information. Three different versions
       *  for three different platforms are implemented.
       */
    void CleanChildren();

      /** This function negotiates a presentation context which will be used by this application
       *  and the other DICOM application that requests an association.
       *  @param assoc The association (network connection to another DICOM application).
       */
    OFCondition NegotiateAssociation( T_ASC_Association *assoc );

      /** This function adds a process to the table that stores process information.
       *  @param pid   the process id of the sub-process which was just started.
       *  @param assoc The association (network connection to another DICOM application).
       */
    void AddProcessToTable( int pid, T_ASC_Association *assoc );

      /** This function removes one particular item from the table which stores all subprocess
       *  information. The item which shall be deleted will be identified by its process id.
       *  @param pid process id.
       */
    void RemoveProcessFromTable( int pid );

      /** This function takes care of refusing an association request.
       *  @param assoc  The association (network connection to another DICOM application).
       *  @param reason The reason why the association request will be refused.
       */
    void RefuseAssociation( T_ASC_Association **assoc, WlmRefuseReasonType reason );

      /** This function takes care of handling the other DICOM application's request. After
       *  having accomplished all necessary steps, the association will be dropped and destroyed.
       *  @param assoc The association (network connection to another DICOM application).
       */
    void HandleAssociation( T_ASC_Association *assoc );

      /** This function takes care of handling the other DICOM application's request.
       *  @param assoc The association (network connection to another DICOM application).
       *  @return An OFCondition value 'cond' for which 'cond.bad()' will always be set
       *          indicating that either some kind of error occurred, or that the peer aborted
       *          the association (DUL_PEERABORTEDASSOCIATION), or that the peer requested the
       *          release of the association (DUL_PEERREQUESTEDRELEASE).
       */
    OFCondition ReceiveAndHandleCommands( T_ASC_Association *assoc );

      /** Having received a DIMSE C-ECHO-RQ message, this function takes care of sending a
       *  DIMSE C-ECHO-RSP message over the network connection.
       *  @param assoc  The association (network connection to another DICOM application).
       *  @param req    The DIMSE C-ECHO-RQ message that was received.
       *  @param presId The ID of the presentation context which was specified in the PDV
       *                which contained the DIMSE command.
       *  @return OFCondition value denoting success or error.
       */
    OFCondition HandleEchoSCP( T_ASC_Association *assoc, T_DIMSE_C_EchoRQ *req, T_ASC_PresentationContextID presId );

      /** This function processes a DIMSE C-FIND-RQ command that was
       *  received over the network connection.
       *  @param assoc   The association (network connection to another DICOM application).
       *  @param request The DIMSE C-FIND-RQ message that was received.
       *  @param presID  The ID of the presentation context which was specified in the PDV
       *                 which contained the DIMSE command.
       *  @return OFCondition value denoting success or error.
       */
    OFCondition HandleFindSCP( T_ASC_Association *assoc, T_DIMSE_C_FindRQ *request, T_ASC_PresentationContextID presID );

      /** Protected undefined copy-constructor. Shall never be called.
       *  @param Src Source object.
       */
    WlmActivityManager( const WlmActivityManager &Src );

      /** Protected undefined operator=. Shall never be called.
       *  @param Src Source object.
       *  @return Reference to this.
       */
    WlmActivityManager &operator=( const WlmActivityManager &Src );


  public:
      /** constructor.
       *  @param dataSourcev                         Pointer to the data source which shall be used.
       *  @param opt_portv                           The port on which the application is supposed to listen.
       *  @param opt_refuseAssociationv              Specifies if an association shall always be refused by the SCP.
       *  @param opt_rejectWithoutImplementationUIDv Specifies if the application shall reject an association if no implementation class UID is provided by the calling SCU.
       *  @param opt_sleepBeforeFindReqv             Specifies how many seconds the application is supposed to sleep before handling a C-FIND-Req.
       *  @param opt_sleepAfterFindv                 Specifies how many seconds the application is supposed to sleep after having handled a C-FIND-Rsp.
       *  @param opt_sleepDuringFindv                Specifies how many seconds the application is supposed to sleep during the handling of a C-FIND-Rsp.
       *  @param opt_maxPDUv                         Maximum length of a PDU that can be received in bytes.
       *  @param opt_networkTransferSyntaxv          Specifies the preferred network transfer syntaxes.
       *  @param opt_failInvalidQueryv               Specifies if the application shall fail on an invalid query.
       *  @param opt_singleProcessv                  Specifies if the application shall run in a single process.
       *  @param opt_maxAssociationsv                Specifies many concurrent associations the application shall be able to handle.
       *  @param opt_blockModev                      Specifies the blocking mode for DIMSE operations
       *  @param opt_dimse_timeoutv                  Specifies the timeout for DIMSE operations
       *  @param opt_acse_timeoutv                   Specifies the timeout for ACSE operations
       *  @param opt_forkedChild                     Indicates, whether this process was "forked" from a parent process, default: false
       *  @param argcv                               Number of commandline arguments given
       *  @param argvv                               Complete command line
       */
    WlmActivityManager(
        WlmDataSource *dataSourcev,
        OFCmdUnsignedInt opt_portv,
        OFBool opt_refuseAssociationv,
        OFBool opt_rejectWithoutImplementationUIDv,
        OFCmdUnsignedInt opt_sleepBeforeFindReqv,
        OFCmdUnsignedInt opt_sleepAfterFindv,
        OFCmdUnsignedInt opt_sleepDuringFindv,
        OFCmdUnsignedInt opt_maxPDUv,
        E_TransferSyntax opt_networkTransferSyntaxv,
        OFBool opt_failInvalidQueryv,
        OFBool opt_singleProcessv,
        int opt_maxAssociationsv,
        T_DIMSE_BlockingMode opt_blockModev,
        int opt_dimse_timeoutv,
        int opt_acse_timeoutv,
        OFBool opt_forkedChildv = OFFalse,
        int argcv = 0,
				const char *argvv[] = NULL );

      /** destructor
       */
    ~WlmActivityManager();

      /** Starts providing the implemented service for calling SCUs.
       *  After having created an instance of this class, this function
       *  shall be called from main.
       *  @return Value that is supposed to be returned from main().
       */
    OFCondition StartProvidingService();

      /** Set directory to store request files to. If set to empty path (default),
       *  request files are not stored.
       *  @param path   Path to directory where request files should be stored to.
       *                Must exist and be writable for worklist application.
       *  @param format The format used for the request file names.
       *                Several placeholders can be used by (denoted by #):<br>
       *                \#a: calling application entity title of the peer SCU<br>
       *                \#c: called application entity title (AE title of worklist SCP application)<br>
       *                \#i: process id of the worklist SCP application process handling the request<br>
       *                \#p: patient ID if present, otherwise empty string<br>
       *                \#t: timestamp in the format YYYYMMDDhhmmssffffff<br>
       *                Default is #t.dump.
       *  @return       OFTrue if path is accepted, OFFalse otherwise
       */
    OFBool setRequestFilePath(const OFString& path="", const OFString& format="#t.dump");
};

#endif
