/*********************************************************************
 *                                                                   *
 * MODULE NAME :  debugwin.c             AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-20-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGDROP' drag/drop sample program.                  *
 *                                                                   *
 *  This module creates listbox windows that are used to display     *
 *  Drag/Drop messages and structure contents.                       *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 * FUNCTIONS AVALABLE TO OTHER MODULES:                              *
 *                                                                   *
 *   dbgCreateWin                                                    *
 *   dbgInsert                                                       *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  07-20-93 - Program coded.                                        *
 *                                                                   *
 *  Rick Fishman                                                     *
 *  Code Blazers, Inc.                                               *
 *  4113 Apricot                                                     *
 *  Irvine, CA. 92720                                                *
 *  CIS ID: 72251,750                                                *
 *                                                                   *
 *********************************************************************/

#pragma strings(readonly)   // used for debug version of memory mgmt routines

/*********************************************************************/
/*------- Include relevant sections of the OS/2 header files --------*/
/*********************************************************************/

#define  INCL_DEV
#define  INCL_SPL
#define  INCL_SPLDOSPRINT
#define  INCL_WINDIALOGS
#define  INCL_WINERRORS
#define  INCL_WINFRAMEMGR
#define  INCL_WININPUT
#define  INCL_WINLISTBOXES
#define  INCL_WINSHELLDATA
#define  INCL_WINSTDCNR
#define  INCL_WINSTDDRAG
#define  INCL_WINSTDFILE
#define  INCL_WINWINDOWMGR
#define  INCL_WINSYS

/**********************************************************************/
/*----------------------------- INCLUDES -----------------------------*/
/**********************************************************************/

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drgdrop.h"

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define MESSAGE_SIZE    1024

#define PRINT_DOCNAME   "Drag/Drop output"

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/


/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/

void Save( HWND hwndFrame );
void Print( HWND hwndFrame );

FNWP wpDbgFrame;

/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/

PFNWP pfnwpFrame;

char szSaveFileName[ CCHMAXPATH ];

/**********************************************************************/
/*--------------------------- dbgCreateWin ---------------------------*/
/*                                                                    */
/*  CREATE THE DEBUG LISTBOX WINDOW                                   */
/*                                                                    */
/*  PARMS: title for the debug window                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: HWND of debug window or NULL if unsuccessful             */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
HWND dbgCreateWin( PSZ szTitle )
{
    FRAMECDATA  fcdata;
    HWND        hwndFrame, hwndLB;

    (void) memset( &fcdata, 0, sizeof( FRAMECDATA ) );

    fcdata.cb            = sizeof( FRAMECDATA );
    fcdata.flCreateFlags = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
                           FCF_MINMAX | FCF_MENU;
    fcdata.idResources   = ID_DEBUGWIN_RESOURCES;

    hwndFrame = WinCreateWindow( HWND_DESKTOP, WC_FRAME, NULL,
                                 FS_NOBYTEALIGN | WS_CLIPCHILDREN,
                                 0, 0, 0, 0, NULLHANDLE,
                                 HWND_TOP, 1, &fcdata, NULL );
    if( hwndFrame )
    {
        hwndLB = WinCreateWindow( hwndFrame, WC_LISTBOX, NULL,
                                  WS_VISIBLE | LS_HORZSCROLL | LS_NOADJUSTPOS,
                                  0, 0, 0, 0, hwndFrame, HWND_TOP, FID_CLIENT,
                                  NULL, NULL );
        if( hwndLB )
        {
            pfnwpFrame = WinSubclassWindow( hwndFrame, wpDbgFrame );

            if( pfnwpFrame )
            {
                WinSetPresParam( hwndLB, PP_FONTNAMESIZE,
                                 strlen( DEBUG_WINDOW_FONT ) + 1,
                                 DEBUG_WINDOW_FONT );
                WinSendMsg( hwndFrame, WM_UPDATEFRAME, NULL, NULL );
                WinSetWindowText( hwndFrame, szTitle );
            }
            else
            {
                Msg( "Subclass of debug window failed. RC: %X",HWNDERR(hwndLB));
                WinDestroyWindow( hwndFrame );
                hwndFrame = NULLHANDLE;
            }
        }
        else
        {
            Msg( "Listbox debug window creation failed. RC: %X",
                 HWNDERR( hwndFrame ) );
            WinDestroyWindow( hwndFrame );
            hwndFrame = NULLHANDLE;
        }
    }
    else
        Msg( "Listbox debug window frame creation failed. RC: %X", HABERR(0) );

    return hwndFrame;
}

/**********************************************************************/
/*---------------------------- dbgInsert -----------------------------*/
/*                                                                    */
/*  INSERT A DEBUG MESSAGE INTO THE DEBUG LISTBOX                     */
/*                                                                    */
/*  PARMS: debug window handle,                                       */
/*         a message in printf format with its parms                  */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: index of inserted item                                   */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
SHORT dbgInsert( HWND hwndDbg, PSZ szFormat,... )
{
    SHORT   sLastItem;
    PSZ     szMsg;
    va_list argptr;

    if( (szMsg = (PSZ) malloc( MESSAGE_SIZE )) == NULL )
    {
        DosBeep( 1000, 1000 );
        return FALSE;
    }

    va_start( argptr, szFormat );
    vsprintf( szMsg, szFormat, argptr );
    va_end( argptr );

    szMsg[ MESSAGE_SIZE - 1 ] = 0;

    sLastItem = (SHORT) WinSendMsg( WinWindowFromID( hwndDbg, FID_CLIENT ),
                                    LM_INSERTITEM, MPFROMSHORT( LIT_END ),
                                    MPFROMP( szMsg ) );

    if( szMsg )
        free( szMsg );

    if( dlgInfo.fScrollToBottom )
        WinSendDlgItemMsg( hwndDbg, FID_CLIENT, LM_SETTOPINDEX,
                           MPFROMSHORT( sLastItem ), NULL );

    return sLastItem;
}

/**********************************************************************/
/*--------------------------- wpDbgFrame -----------------------------*/
/*                                                                    */
/*  SUBCLASSED FRAME WINDOW PROCEDURE.                                */
/*                                                                    */
/*  PARMS: standard window procedure parameters                       */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EXPENTRY wpDbgFrame( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    switch( msg )
    {
        case WM_SYSCOMMAND:
            if( SHORT1FROMMP( mp1 ) == SC_CLOSE )
            {
                WinDestroyWindow( hwnd );
                return 0;
            }

            break;

        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 ) )
            {
                case IDM_DEBUGWIN_SAVE:
                    Save( hwnd );
                    return 0;

                case IDM_DEBUGWIN_PRINT:
                    Print( hwnd );
                    return 0;

                case IDM_DEBUGWIN_CLEAR:
                    WinSendDlgItemMsg( hwnd, FID_CLIENT, LM_DELETEALL, NULL,
                                       NULL );
                    return 0;
            }

            break;
    }

    return pfnwpFrame( hwnd, msg, mp1, mp2 );
}

/**********************************************************************/
/*------------------------------ Save --------------------------------*/
/*                                                                    */
/*  SAVE THE LISTBOX CONTENTS TO A FILE.                              */
/*                                                                    */
/*  PARMS: Debug window's frame window handle                         */
/*                                                                    */
/*  NOTES: This one function requires *lots* of stack space!          */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void Save( HWND hwndFrame )
{
    FILEDLG fd;

    (void) memset( &fd, 0, sizeof( FILEDLG ) );
    fd.cbSize   = sizeof( FILEDLG );
    fd.fl       = FDS_CENTER | FDS_SAVEAS_DIALOG;
    fd.pszTitle = "Please select a FileName";
    strcpy( fd.szFullFile, szSaveFileName );

    WinFileDlg( HWND_DESKTOP, hwndFrame, &fd );

    if( *(fd.szFullFile) && fd.lReturn == DID_OK )
    {
        HWND  hwndLB = WinWindowFromID( hwndFrame, FID_CLIENT );
        SHORT cItems;
        int   i;
        char  szItem[ MANUAL_RMF_LEN + 2 ];   // This would be the longest item
        FILE  *fh = fopen( fd.szFullFile, "w" );

        if( fh )
        {
            // Store it for the next time the user wants to save

            strcpy( szSaveFileName, fd.szFullFile );

            cItems = (SHORT) WinSendMsg( hwndLB, LM_QUERYITEMCOUNT, NULL, NULL);
            for( i = 0; i < cItems; i++ )
            {
                if( WinSendMsg( hwndLB, LM_QUERYITEMTEXT,
                                MPFROM2SHORT( i, sizeof szItem ),
                                MPFROMP( szItem ) ) )
                {
                    strcat( szItem, "\n" );
                    if( fputs( szItem, fh ) == EOF )
                    {
                        Msg( "Out of disk space!" );
                        break;
                    }
                }
            }

            fclose( fh );
        }
    }
}

/**********************************************************************/
/*------------------------------ Print -------------------------------*/
/*                                                                    */
/*  PRINT THE LISTBOX CONTENTS TO THE DEFAULT PRINTER.                */
/*                                                                    */
/*  PARMS: Debug window's frame window handle                         */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void Print( HWND hwndFrame )
{
    DEVOPENSTRUC dop = { "LPT1Q", "IBM4201", 0, "PM_Q_RAW", 0, 0, 0, 0, 0 };
    PPRQINFO3    pprq3 = NULL;
    char         szBuf[ 256 ];
    char         szItem[ MANUAL_RMF_LEN + 2 ]; // This would be the longest item
    USHORT       usSize = (SHORT) PrfQueryProfileString( HINI_PROFILE,
                                        SPL_INI_SPOOLER, "QUEUE", NULL, szBuf,
                                        (LONG) sizeof( szBuf ) );
    if( usSize )
    {
        HWND   hwndLB = WinWindowFromID( hwndFrame, FID_CLIENT );
        ULONG  cbNeeded;
        APIRET rc;
        PCH    pchDot;
        HSPL   hspl;

        szBuf[ usSize - 2 ] = 0;    // get rid of terminating semicolon

        dop.pszLogAddress = szBuf;  // default queue

        SplQueryQueue( NULL, szBuf, 3, NULL, 0, &cbNeeded );

        pprq3 = (PPRQINFO3) malloc( cbNeeded );
        if( !pprq3 )
        {
            Msg( "Out of memory" );
            return;
        }

        rc = SplQueryQueue( NULL, szBuf, 3, pprq3, cbNeeded, &cbNeeded );

        if( rc )
        {
            Msg( "SplQueryQueue RC: %u", rc );
            return;
        }

        pchDot = strchr( pprq3->pszDriverName, '.' );

        if( pchDot )
            *pchDot = 0;

        dop.pszDriverName = pprq3->pszDriverName;
        dop.pdriv = pprq3->pDriverData;

        // Open the Print Manager

        if( !(hspl = SplQmOpen( "*", 4L, (PQMOPENDATA) &dop )) )
            Msg( "SplQmOpen failed\n" );

        // Start the spool file and name it

        if( hspl && !(SplQmStartDoc( hspl, PRINT_DOCNAME )) )
            Msg( "SplQmStartDoc failed\n" );

        // Write to the spool file

        if( hspl )
        {
            int i;
            int cItems = (int) WinSendMsg( hwndLB,LM_QUERYITEMCOUNT,NULL,NULL );
            for( i = 0; i < cItems; i++ )
            {
                if( WinSendMsg( hwndLB, LM_QUERYITEMTEXT,
                                MPFROM2SHORT( i, sizeof szItem ),
                                MPFROMP( szItem ) ) )
                {
                    strcat( szItem, "\n\r" );
                    if( !SplQmWrite( hspl, strlen( szItem ), szItem ) )
                        Msg( "SplQmWrite failed\n" );
                }
            }

        }

        // End the spool file. This starts it printing.

        if( hspl && !SplQmEndDoc( hspl ) )
            Msg( "SplQmEndDoc failed\n" );

        // Close the Print Manager

        if( hspl && !SplQmClose( hspl ) )
            Msg( "SplQmClose failed\n" );
    }
    else
        Msg( "PrfQueryProfileString for default queue failed\n" );

}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
