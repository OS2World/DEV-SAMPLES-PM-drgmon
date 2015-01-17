/*********************************************************************
 *                                                                   *
 * MODULE NAME :  show.c                 AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-21-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGDROP' drag/drop sample program.                  *
 *                                                                   *
 *  This module takes Drag/Drop structures and displays their        *
 *  contents in a debug window.                                      *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 *                                                                   *
 * FUNCTIONS AVALABLE TO OTHER MODULES:                              *
 *                                                                   *
 *   showDragoverNotifyInfo                                          *
 *   showDragInfo                                                    *
 *   showDragTransfer                                                *
 *   showDragItem                                                    *
 *   showRenderReply                                                 *
 *   showPrintReply                                                  *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  07-21-93 - Program coded.                                        *
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

#define  INCL_SHLERRORS
#define  INCL_WINERRORS
#define  INCL_WINFRAMEMGR
#define  INCL_WINSTDCNR
#define  INCL_WINSTDDRAG
#define  INCL_WINWINDOWMGR

/**********************************************************************/
/*----------------------------- INCLUDES -----------------------------*/
/**********************************************************************/

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drgdrop.h"

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/


/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/


/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/


/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/


/**********************************************************************/
/*--------------------- showDragoverNotifyInfo -----------------------*/
/*                                                                    */
/*  SHOW THE INFO ON A DM_DRAGOVERNOTIFY MESSAGE.                     */
/*                                                                    */
/*  PARMS: frame window handle whose debug window we will write to,   */
/*         pointer to a DRAGINFO structure,                           */
/*         the target's reply                                         */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void showDragoverNotifyInfo( HWND hwndFrame, PDRAGINFO pDragInfo, MRESULT mr )
{
    PINSTANCE pi = INSTDATA( hwndFrame );

    if( pi )
    {
        HWND  hDbg = pi->hwndDebug;
        int   i;
        char  szBuf[ 100 ];
        ULONG ulDrop = SHORT1FROMMR( mr ), ulOp = SHORT2FROMMR( mr );

        if( !hDbg || !WinIsWindow( ANCHOR( hwndFrame ), hDbg ) )
            return;

        if( dlgInfo.fOnlyMessageNames )
        {
            dbgInsert( hDbg, "DM_DRAGOVERNOTIFY" );
            return;
        }

        showDragInfo( hwndFrame, "DM_DRAGOVERNOTIFY", pDragInfo );

        strcpy( szBuf, "Target replied [" );

        for( i = 0; i < cDragoverReplyTypes; i++ )
            if( ulDrop == dcDragoverReply[ i ].iItem )
            {
                strcat( szBuf, dcDragoverReply[ i ].szItem );
                break;
            }

        if( i == cDragoverReplyTypes )
            strcat( szBuf, "0" );

        strcat( szBuf, "," );

        for( i = 0; i < cOperations; i++ )
            if( ulOp == dcOperation[ i ].iItem )
            {
                strcat( szBuf, dcOperation[ i ].szItem );
                break;
            }

        if( i == cOperations )
            strcat( szBuf, "0" );

        strcat( szBuf, "] to DM_DRAGOVER" );

        dbgInsert( hDbg, szBuf );
    }
    else
        Msg( "showDragoverNotifyInfo cant get Inst data RC(%X)",
             HWNDERR( hwndFrame ) );
}

/**********************************************************************/
/*-------------------------- showDragInfo ----------------------------*/
/*                                                                    */
/*  DISPLAY THE CONTENTS OF A DRAGINFO STRUCTURE.                     */
/*                                                                    */
/*  PARMS: frame window handle whose debug window we will write to,   */
/*         string that uniquely identifies the operation going on,    */
/*         pointer to a DRAGINFO structure                            */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void showDragInfo( HWND hwndFrame, PSZ pszCurrentOp, PDRAGINFO pDragInfo )
{
    PINSTANCE pi = INSTDATA( hwndFrame );

    if( pi )
    {
        HWND hDbg = pi->hwndDebug;
        int  i;
        char hstrBuf[ 100 ];

        if( !hDbg || !WinIsWindow( ANCHOR( hwndFrame ), hDbg ) )
            return;

        if( dlgInfo.fOnlyMessageNames )
        {
            dbgInsert( hDbg, pszCurrentOp );
            return;
        }

        sprintf( hstrBuf, " %s ", pszCurrentOp );
        dbgInsert( hDbg, hstrBuf );

        dbgInsert( hDbg, "Drag Items: %u", pDragInfo->cditem );

        *hstrBuf = 0;

        for( i = 0; i < cOperations; i++ )
            if( pDragInfo->usOperation == dcOperation[ i ].iItem )
            {
                strcpy( hstrBuf, dcOperation[ i ].szItem );
                break;
            }

        dbgInsert( hDbg, "Operation: %X [%s]", pDragInfo->usOperation, hstrBuf);
        dbgInsert( hDbg, "Source HWND: %p", pDragInfo->hwndSource );
        dbgInsert( hDbg, "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�" );

        if( dlgInfo.fOnlyFirstItem || pDragInfo->cditem == 1 )
            showDragItem( hwndFrame, pszCurrentOp,
                          DrgQueryDragitemPtr( pDragInfo, 0 ), -1 );
        else
        {
            for( i = 0; i < pDragInfo->cditem; i++ )
            {
                showDragItem( hwndFrame, pszCurrentOp,
                              DrgQueryDragitemPtr( pDragInfo, i ), i + 1 );

                if( i != (pDragInfo->cditem - 1) )
                    dbgInsert( hDbg, "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�" );
            }

            dbgInsert( hDbg, " " );
        }
    }
    else
        Msg( "showDragInfo cant get Inst data RC(%X)", HWNDERR( hwndFrame ) );
}

/**********************************************************************/
/*------------------------- showDragTransfer -------------------------*/
/*                                                                    */
/*  SEND DRAGTRANSFER INFO TO THE DEBUG WINDOW.                       */
/*                                                                    */
/*  PARMS: frame window handle whose debug window we will write to,   */
/*         string that uniquely identifies the operation going on,    */
/*         pointer to a DRAGTRANSFER structure                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void showDragTransfer( HWND hwndFrame, PSZ pszCurrentOp,
                       PDRAGTRANSFER pDragXfer )
{
    PINSTANCE pi = INSTDATA( hwndFrame );

    if( pi )
    {
        HWND hDbg = pi->hwndDebug;
        int  i;
        char hstrBuf[ 100 ];

        if( !hDbg || !WinIsWindow( ANCHOR( hwndFrame ), hDbg ) )
            return;

        if( dlgInfo.fOnlyMessageNames )
        {
            dbgInsert( hDbg, pszCurrentOp );
            return;
        }

        sprintf( hstrBuf, " %s ", pszCurrentOp );
        dbgInsert( hDbg, hstrBuf );

        dbgInsert( hDbg, "hwndClient: %p", pDragXfer->hwndClient );

        dbgInsert( hDbg, "컴컴� DRAGITEM IN DRAGTRANSFER 컴컴�" );
        showDragItem( hwndFrame, pszCurrentOp, pDragXfer->pditem, -1 );
        dbgInsert( hDbg, "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴" );

        DrgQueryStrName( pDragXfer->hstrSelectedRMF, sizeof( hstrBuf ),
                         hstrBuf );
        dbgInsert( hDbg, "hstrSelectedRMF: %s", hstrBuf );

        DrgQueryStrName( pDragXfer->hstrRenderToName, sizeof( hstrBuf ),
                         hstrBuf );
        dbgInsert( hDbg, "hstrRenderToName: %s", hstrBuf );

        dbgInsert( hDbg, "ulTargetInfo: %u", pDragXfer->ulTargetInfo );

        *hstrBuf = 0;

        for( i = 0; i < cOperations; i++ )
            if( pDragXfer->usOperation & dcOperation[ i ].iItem )
            {
                strcat( hstrBuf, dcOperation[ i ].szItem );
                strcat( hstrBuf, " " );
            }

        dbgInsert( hDbg, "usOperation: %X [%s]", pDragXfer->usOperation,
                   hstrBuf );

        *hstrBuf = 0;

        for( i = 0; i < cRenderReplyTypes; i++ )
            if( pDragXfer->fsReply & dcRenderReply[ i ].iItem )
            {
                strcat( hstrBuf, dcRenderReply[ i ].szItem );
                strcat( hstrBuf, " " );
            }

        dbgInsert( hDbg, "fsReply: %X [%s]", pDragXfer->fsReply, hstrBuf );
        dbgInsert( hDbg, "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴" );
        dbgInsert( hDbg, " " );
    }
}

/**********************************************************************/
/*--------------------------- showDragItem ---------------------------*/
/*                                                                    */
/*  SEND DRAGITEM INFO TO THE DEBUG WINDOW.                           */
/*                                                                    */
/*  PARMS: frame window handle whose debug window we will write to,   */
/*         string that uniquely identifies the operation going on,    */
/*         pointer to the DRAGITEM structure,                         */
/*         positional 1-based offset of DRAGITEM in array of DRAGITEMS*/
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void showDragItem( HWND hwndFrame, PSZ pszCurrentOp, PDRAGITEM pDragItem,
                   int iOffset )
{
    PINSTANCE pi = INSTDATA( hwndFrame );

    if( pi )
    {
        HWND hDbg = pi->hwndDebug;
        int  i;
        char hstrBuf[ 100 ];
        char szOffset[ 6 ];

        if( !hDbg || !WinIsWindow( ANCHOR( hwndFrame ), hDbg ) )
            return;

        if( dlgInfo.fOnlyMessageNames )
        {
            dbgInsert( hDbg, pszCurrentOp );
            return;
        }

        // Take care of showing the offset if applicable. If this display is
        // part of a global DRAGINFO display, the caller will take care of
        // any border drawing. All callers outside of this module will call
        // with a 0 as the last parameter. If called as part of a DRAGTRANSFER,
        // the last parameter will by -1.

        if( iOffset == 0 )
        {
            strcpy( szOffset, ":" );
            sprintf( hstrBuf, " %s ", pszCurrentOp );
            dbgInsert( hDbg, hstrBuf );
        }
        else if( iOffset == -1 )
            strcpy( szOffset, ":" );
        else
            sprintf( szOffset, "[%d]:", iOffset );

        dbgInsert( hDbg, "hwndItem%s %p, ulItemID%s %X", szOffset,
                   pDragItem->hwndItem, szOffset, pDragItem->ulItemID );

        DrgQueryStrName( pDragItem->hstrType, sizeof( hstrBuf ), hstrBuf );
        dbgInsert( hDbg, "Type%s %s", szOffset, hstrBuf );

        DrgQueryStrName( pDragItem->hstrRMF, sizeof( hstrBuf ), hstrBuf );
        dbgInsert( hDbg, "RMF%s %s", szOffset, hstrBuf );

        DrgQueryStrName( pDragItem->hstrContainerName, sizeof( hstrBuf ),
                         hstrBuf );
        dbgInsert( hDbg, "CnrName%s %s", szOffset, hstrBuf );

        DrgQueryStrName( pDragItem->hstrSourceName, sizeof( hstrBuf ),
                         hstrBuf );
        dbgInsert( hDbg, "Source%s %s", szOffset, hstrBuf );

        DrgQueryStrName( pDragItem->hstrTargetName, sizeof( hstrBuf ),
                         hstrBuf );
        dbgInsert( hDbg, "Target%s %s", szOffset, hstrBuf );

        dbgInsert( hDbg, "cxOffset%s: %d", szOffset, pDragItem->cxOffset );
        dbgInsert( hDbg, "cyOffset%s: %d", szOffset, pDragItem->cyOffset );

        *hstrBuf = 0;

        for( i = 0; i < cControlTypes; i++ )
            if( pDragItem->fsControl & dcControl[ i ].iItem )
            {
                strcat( hstrBuf, dcControl[ i ].szItem );
                strcat( hstrBuf, " " );
            }

        dbgInsert( hDbg, "Control Flags%s %X [%s]", szOffset,
                   pDragItem->fsControl, hstrBuf );

        *hstrBuf = 0;

        for( i = 0; i < cSupportedOps; i++ )
            if( pDragItem->fsSupportedOps & dcSupportedOp[ i ].iItem )
            {
                strcat( hstrBuf, dcSupportedOp[ i ].szItem );
                strcat( hstrBuf, " " );
            }

        dbgInsert( hDbg, "Supported Ops%s %X [%s]", szOffset,
                   pDragItem->fsSupportedOps, hstrBuf );

        // If this display is part of a global DRAGINFO display, the caller
        // will be drawing borders. Same with DRAGTRANSFER.

        if( iOffset == 0 )
        {
            dbgInsert( hDbg, "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�" );
            dbgInsert( hDbg, " " );
        }
    }
    else
        Msg( "showDragItem cant get Inst data RC(%X)", HWNDERR( hwndFrame ) );
}

/**********************************************************************/
/*------------------------- showRenderReply --------------------------*/
/*                                                                    */
/*  SHOW THE REPLY TO A DRAGTRANSFER MESSAGE.                         */
/*                                                                    */
/*  PARMS: frame window handle whose debug window we will write to,   */
/*         string that will prefix the reply,                         */
/*         the reply                                                  */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void showRenderReply( HWND hwndFrame, PSZ pszPrefix, ULONG flReply )
{
    PINSTANCE pi = INSTDATA( hwndFrame );

    if( pi )
    {
        HWND hDbg = pi->hwndDebug;
        int  i;
        char szBuf[ 100 ];

        if( !hDbg || !WinIsWindow( ANCHOR( hwndFrame ), hDbg ) )
            return;

        *szBuf = 0;

        for( i = 0; i < cRenderReplyTypes; i++ )
            if( flReply & dcRenderReply[ i ].iItem )
            {
                strcat( szBuf, dcRenderReply[ i ].szItem );
                strcat( szBuf, " " );
            }

        dbgInsert( hDbg, "%s%X [%s]", pszPrefix, flReply, szBuf );
    }
    else
        Msg( "showRenderReply cant get Inst data RC(%X)", HWNDERR( hwndFrame ));
}

/**********************************************************************/
/*------------------------- showPrintReply ---------------------------*/
/*                                                                    */
/*  SHOW THE REPLY TO A DM_PRINTOBJECT or DM_DISCARDOBJECT MESSAGE.   */
/*                                                                    */
/*  PARMS: frame window handle whose debug window we will write to,   */
/*         string that will prefix the reply,                         */
/*         the reply                                                  */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void showPrintReply( HWND hwndFrame, PSZ pszPrefix, ULONG flReply )
{
    PINSTANCE pi = INSTDATA( hwndFrame );

    if( pi )
    {
        HWND hDbg = pi->hwndDebug;
        int  i;
        char szBuf[ 100 ];

        if( !hDbg || !WinIsWindow( ANCHOR( hwndFrame ), hDbg ) )
            return;

        for( i = 0; i < cPrintReplyTypes; i++ )
            if( flReply & dcPrintReply[ i ].iItem )
            {
                strcpy( szBuf, dcPrintReply[ i ].szItem );
                break;
            }

        dbgInsert( hDbg, "%s%X [%s]", pszPrefix, flReply, szBuf );
    }
    else
        Msg( "showPrintReply cant get Inst data RC(%X)", HWNDERR( hwndFrame ) );
}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
