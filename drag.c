/*********************************************************************
 *                                                                   *
 * MODULE NAME :  drag.c                 AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-20-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGDROP' drag/drop sample program.                  *
 *                                                                   *
 *  This module handles all the Drag/Drop processing for the         *
 *  DRGDROP.EXE sample program.                                      *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 *  There is no source rendering done in this sample program. For a  *
 *  sample that does that, get my DRGRENDR.EXE sample, or            *
 *  DRGTHRND.EXE that does rendering in secondary threads.           *
 *                                                                   *
 * FUNCTIONS AVALABLE TO OTHER MODULES:                              *
 *                                                                   *
 *   dragMessage                                                     *
 *   dragInit                                                        *
 *   dragOver                                                        *
 *   dragLeave                                                       *
 *   dragDrop                                                        *
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
#define  INCL_DOSERRORS
#define  INCL_DOSPROCESS
#define  INCL_SHLERRORS
#define  INCL_WINDIALOGS
#define  INCL_WINERRORS
#define  INCL_WINFRAMEMGR
#define  INCL_WININPUT
#define  INCL_WINPOINTERS
#define  INCL_WINSTDCNR
#define  INCL_WINSTDDRAG
#define  INCL_WINWINDOWMGR

/**********************************************************************/
/*----------------------------- INCLUDES -----------------------------*/
/**********************************************************************/

#include <os2.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drgdrop.h"

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define DOC_NAME  "DrgDrop"

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/


/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/

int     CountSelectedRecs   ( HWND hwndFrame, PCNRREC pCnrRecUnderMouse,
                              PBOOL pfUseSelectedRecs );
void    SetSelectedDragItems( HWND hwndFrame, int cRecs, PDRAGINFO pDragInfo,
                              PDRAGIMAGE pDragImage );
void    SetOneDragItem      ( HWND hwndFrame, PCNRREC pCnrRecUnderMouse,
                              PDRAGINFO pDragInfo, PDRAGIMAGE pDragImage,
                              int iOffset );
USHORT  DetermineDefaultOp  ( PDRAGINFO pDragInfo );
void    ProcessDroppedItem  ( HWND hwndFrame, PDRAGITEM pDragItem );
MRESULT DiscardObjects      ( HWND hwndFrame, PDRAGINFO pDragInfo );
MRESULT PrintObject         ( HWND hwndFrame, PDRAGITEM pDragItem,
                              PPRINTDEST pPrintDest );
MRESULT EndConversation     ( HWND hwndFrame, PCNRREC pCnrRec, ULONG flSuccess);
void    DragoverNotify      ( HWND hwndFrame, PDRAGINFO pDraginfo, MRESULT mr );
void    RemoveSourceEmphasis( HWND hwndFrame );

FNWP wpSource, wpTarget;

/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/

extern char szDragCnrTitle[];

/**********************************************************************/
/*--------------------------- dragMessage ----------------------------*/
/*                                                                    */
/*  A DM_ MESSAGE WAS RECEIVED BY THE FRAME WINDOW PROCEDURE.         */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         message id,                                                */
/*         mp1 of the message,                                        */
/*         mp2 of the message                                         */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: return code of message                                   */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT dragMessage( HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    switch( msg )
    {
        case DM_ENDCONVERSATION: // The SOURCE window gets this message
            return EndConversation( hwndFrame, (PCNRREC) mp1, (ULONG) mp2 );

        case DM_PRINTOBJECT:
            return PrintObject( hwndFrame, (PDRAGITEM) mp1, (PPRINTDEST) mp2 );

        case DM_DISCARDOBJECT:
            return DiscardObjects( hwndFrame, (PDRAGINFO) mp1 );

        // The rest of these messages aren't really processed in this program.
        // They are just used to display the contents of the Drag/Drop
        // structures and to minimally process the message to allow the
        // Drag/Drop operation to continue. None of the DrgDragFiles() messages
        // are processed in this program.

        case DM_DRAGOVERNOTIFY:
            DragoverNotify( hwndFrame, (PDRAGINFO) mp1, (MRESULT) mp2 );
            return 0;

        case DM_DROPHELP:
            dbgInsert( INSTDATA( hwndFrame )->hwndDebug,
                       "DM_DROPHELP received" );
            return 0;

        case DM_RENDER:
            showDragTransfer( hwndFrame, "DM_RENDER (returning TRUE)",
                              (PDRAGTRANSFER) mp1 );
            return (MRESULT) TRUE;

        case DM_RENDERCOMPLETE:
            showDragTransfer( hwndFrame, "DM_RENDERCOMPLETE",
                             (PDRAGTRANSFER) mp1 );
            return 0;

        case DM_RENDERPREPARE:
            showDragTransfer( hwndFrame, "DM_RENDERPREPARE",
                             (PDRAGTRANSFER) mp1 );
            return (MRESULT) TRUE;
    }

    return 0;
}

/**********************************************************************/
/*----------------------------- dragInit -----------------------------*/
/*                                                                    */
/*  PROCESS CN_INITDRAG NOTIFY MESSAGE.                               */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the CNRDRAGINIT structure                       */
/*                                                                    */
/*  NOTES: the CN_INITDRAG message is equivalent to the WM_BEGINDRAG  */
/*         message. The reason I mention this is because if the source*/
/*         window in the drag is not a container you will get the     */
/*         WM_BEGINDRAG message.                                      */
/*                                                                    */
/*         WM_BEGINDRAG is sent to windows to allow them to know that */
/*         the user is requesting a drag from their window. This      */
/*         message contains only the x,y coordinates of the mouse at  */
/*         the time of WM_BEGINDRAG.                                  */
/*                                                                    */
/*         The reason for CN_INITDRAG is first so that the container  */
/*         can notify its owner when it gets a WM_BEGINDRAG message   */
/*         and second so that the container can give its owner more   */
/*         info, like what container record the mouse pointer is over.*/
/*         To accomplish this, the container packages up this informa-*/
/*         tion in a CNRDRAGINIT structure and sends that structure   */
/*         along with the CN_INITDRAG message.                        */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void dragInit( HWND hwndFrame, PCNRDRAGINIT pcdi )
{
    PCNRREC     pCnrRecUnderMouse = (PCNRREC) pcdi->pRecord;
    PDRAGIMAGE  pDragImage = NULL;
    PDRAGINFO   pDragInfo = NULL;
    BOOL        fUseSelectedRecs;
    PINSTANCE   pi = INSTDATA( hwndFrame );
    int         cRecs;

    if( !pi )
    {
        Msg( "dragInit cant get Inst data RC(%X)", HWNDERR( hwndFrame ) );
        return;
    }

    // pSavedDragInfo is used during drop processing. When the drop is complete
    // (the source gets a DM_ENDCONVERSATION message from the target), the
    // source will free the DRAGINFO structure and set this field to NULL.
    // So pSavedDragInfo is non-NULL if a drop has not yet completed. In this
    // case we make it easy on ourselves by not allowing another drag. The
    // reason for this is that if we allow another drag to take place we will
    // need to overwrite this pSavedDragInfo field in which case the drop
    // processing would not free the right DRAGINFO structure. Obviously in a
    // commercial app you'd want to use a different mechanism for storing the
    // DRAGINFO structure, like a linked list, so you could start another drag
    // while the drop is in progress.

    if( pi->pSavedDragInfo )
    {
        WinAlarm( HWND_DESKTOP, WA_WARNING );
        return;
    }

    // Count the records that have CRA_SELECTED emphasis. Also return whether
    // or not we should process the CRA_SELECTED records. If the container
    // record under the mouse does not have this emphasis, we shouldn't. In that
    // case we would just process the record under the mouse.

    cRecs = CountSelectedRecs( hwndFrame, pCnrRecUnderMouse, &fUseSelectedRecs);

    if( cRecs )
    {
        int iDragImageArraySize = cRecs * sizeof( DRAGIMAGE );

        // Allocate an array of DRAGIMAGE structures. Each structure contains
        // info about an image that will be under the mouse pointer during the
        // drag. This image will represent a container record being dragged.

        pDragImage = (PDRAGIMAGE) malloc( iDragImageArraySize );

        if( pDragImage )
        {
            memset( pDragImage, 0, iDragImageArraySize );

            // Let PM allocate enough memory for a DRAGINFO structure as well
            // as a DRAGITEM structure for each record being dragged. It will
            // allocate shared memory so other processes can participate in the
            // drag/drop.

            pDragInfo = DrgAllocDraginfo( cRecs );

            if( pDragInfo )
            {
                pi->pSavedDragInfo = pDragInfo;
                pi->cDragItems = cRecs;
            }
            else
                Msg( "DrgAllocDraginfo failed. RC(%X)", HWNDERR( hwndFrame ) );
        }
        else
            Msg( "Out of memory in dragInit" );
    }

    if( cRecs && pDragInfo && pDragImage )
    {
        // Make sure the data in the dlgInfo structure is current.

        bookRefreshDlgInfo();

        // Set the data from the container records into the DRAGITEM and
        // DRAGIMAGE structures. If we are to process CRA_SELECTED container
        // records, do them all in one function. If not, pass a pointer to the
        // container record under the mouse to a different function that will
        // fill in just one DRAGITEM / DRAGIMAGE structure.

        if( fUseSelectedRecs )
            SetSelectedDragItems( hwndFrame, cRecs, pDragInfo, pDragImage );
        else
            SetOneDragItem( hwndFrame, pCnrRecUnderMouse, pDragInfo,
                            pDragImage, 0 );

        showDragInfo( hwndFrame, "CN_INITDRAG", pDragInfo );

        // If DrgDrag returns NULLHANDLE, that means the user hit Esc or F1
        // while the drag was going on so the target didn't have a chance to
        // delete the string handles. So it is up to the source window to do
        // it. Unfortunately there doesn't seem to be a way to determine
        // whether the NULLHANDLE means Esc was pressed as opposed to there
        // being an error in the drag operation. So we don't attempt to figure
        // that out. To us, a NULLHANDLE means Esc was pressed...

        if( !DrgDrag( hwndFrame, pDragInfo, pDragImage, cRecs, VK_ENDDRAG,
                      NULL ) )
        {
            if( !DrgDeleteDraginfoStrHandles( pDragInfo ) )
                Msg( "dragInit DrgDeleteDraginfoStrHandles RC(%X)",
                     HWNDERR( hwndFrame ) );

            if( !DrgFreeDraginfo( pDragInfo ) )
                Msg( "dragInit DrgFreeDraginfo RC(%X)", HWNDERR( hwndFrame ) );

            pi->pSavedDragInfo = NULL;
        }

        // Take off source emphasis from the records that were dragged

        RemoveSourceEmphasis( hwndFrame );
    }

    if( pDragImage )
        free( pDragImage );
}

/**********************************************************************/
/*----------------------------- dragOver -----------------------------*/
/*                                                                    */
/*  PROCESS CN_DRAGOVER NOTIFY MESSAGE.                               */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the CNRDRAGINFO structure                       */
/*                                                                    */
/*  NOTES: The container sends the CN_DRAGOVER message to its owner   */
/*         when it gets a DM_DRAGOVER message. The container takes    */
/*         the info it gets on the DM_DRAGOVER message and combines   */
/*         it with other container-specific dragover info into a      */
/*         CNRDRAGINFO structure, then passes a pointer to that       */
/*         structure to its owner in the CN_DRAGOVER message.         */
/*                                                                    */
/*  RETURNS: return value from CN_DRAGOVER processing                 */
/*           1st USHORT (Drop Indicator)                              */
/*                                                                    */
/*             - DOR_DROP      - OK to drop                           */
/*             - DOR_NODROP    - Not OK to drop at this x,y           */
/*             - DOR_NODROPOP  - Not OK to drop because of the        */
/*                               drop operation.                      */
/*             - DOR_NEVERDROP - Not OK to drop and never will be OK  */
/*                               to drop.                             */
/*                                                                    */
/*           2nd USHORT (Default Operation)                           */
/*                                                                    */
/*             - DO_COPY       - 'Copy' is the default operation      */
/*             - DO_LINK       - 'Link' is the default operation      */
/*             - DO_MOVE       - 'Move' is the default operation      */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT dragOver( HWND hwndFrame, PCNRDRAGINFO pcdi )
{
    USHORT    usDrop, usDefaultOp = 0;
    PDRAGINFO pDragInfo = pcdi->pDragInfo;
    PDRAGITEM pDragItem;
    PINSTANCE pi = INSTDATA( hwndFrame );
    int       i;

    if( !pi )
    {
        Msg( "dragOver cant get Inst data RC(%X)", HWNDERR( hwndFrame ) );
        return MRFROM2SHORT( DOR_NEVERDROP, 0 );
    }

    // Note that the default operation doesn't mean anything if usDrop is not
    // DOR_DROP.

    usDrop = 0;

    if( !DrgAccessDraginfo( pDragInfo ) )
    {
        Msg( "dragOver DrgAccessDraginfo RC(%X)", HWNDERR( hwndFrame ) );
        return MRFROM2SHORT( DOR_NEVERDROP, 0 );
    }

    // We only want to display the full DRAGINFO structure in the debug window
    // if it is the first DM_DRAGOVER message while over our window. If we
    // display all of them the drag operation takes a real performance hit. If
    // the drag operation changes (i.e. from a 'copy' to a 'move'), we
    // re-display the DRAGINFO structure. We clear the fDragoverInProgress flag
    // in the DM_DRAGLEAVE message.

    if( pi->fDragoverInProgress )
        if( pi->usDragoverOp == pDragInfo->usOperation )
            dbgInsert( pi->hwndDebug, " *** repeat DM_DRAGOVER" );
        else
        {
            pi->usDragoverOp = pDragInfo->usOperation;
            showDragInfo( hwndFrame, "DM_DRAGOVER", pDragInfo );
        }
    else
    {
        pi->usDragoverOp = pDragInfo->usOperation;
        pi->fDragoverInProgress = TRUE;
        showDragInfo( hwndFrame, "DM_DRAGOVER", pDragInfo );
    }

    // Don't allow a window to drop on itself. Normally a container would allow
    // this but it adds complexity to the drop code because the container would
    // have to be set up without CCS_AUTOPOSITION and it would have to remove
    // and re-insert the record so that it would stay in its proper new position.
    // Code for this is in my CNRADV.ZIP program. Since it doesn't benefit a
    // Drag/Drop sample I left it out of this program.

    if( pDragInfo->hwndSource == hwndFrame )
        usDrop = DOR_NEVERDROP;

    // If the user hasn't changed the default of allowing all drops, we simply
    // return and allow the drop with whatever operation is currently in
    // progress. The reason for allowing all drops is so you can see exactly
    // what would be in the DRAGITEM's and the DRAGINFO on the drop (in the
    // debug window. If the user doesn't change this default in the dialog box,
    // we allow the drop to happen but we don't do anything on the drop on an
    // unacceptable combination except to send a DM_ENDCONVERSATION message with
    // a return code of DMFL_TARGETFAIL.

    if( !usDrop && dlgInfo.fAllowAllDrops )
    {
        usDrop = DOR_DROP;
        usDefaultOp = pDragInfo->usOperation;
    }

    // If the current operation is 'unknown', we don't want to play guessing
    // games so we'll ignore it. We'll allow the user to change the operation
    // with the Shift,Ctrl keys and then we'll go further.

    if( !usDrop && pDragInfo->usOperation == DO_UNKNOWN )
        usDrop = DOR_NODROPOP;

    // We must set the operation that we want to perform on the drop. This may
    // mean changing the operation that the user is currently indicating that
    // they want to do. For instance, we may change a 'move' operation to a
    // 'copy'. We could also decide that we can't drop given the current
    // operation. In any case, if we change the operation, the images being
    // dragged will be changed automatically by PM. For example, if we change
    // the operation from a move to a copy, PM will halftone the images.

    if( !usDrop || usDrop == DOR_DROP )
        usDefaultOp = DetermineDefaultOp( pDragInfo );

    // Check each item. If one fails, we don't allow the drop. Note that you
    // *could* accept the drop if some failed but that just makes the drop
    // more difficult. What you would do is return DMFL_TARGETFAIL on the
    // DM_ENDCONVERSATION message for the items that you cannot support.
    // Normally all items use the same criteria though. So most-times if one
    // item is ok, all items are ok.

    if( !usDrop )
        for( i = 0; i < pDragInfo->cditem; i++ )
        {
            pDragItem = DrgQueryDragitemPtr( pDragInfo, i );

            if( pDragItem )
            {
                // Only allow a drop if the operation is able to be provided.

                if( ((pDragItem->fsSupportedOps & DO_COPYABLE) &&
                                    (usDefaultOp == DO_COPY)) ||
                     ((pDragItem->fsSupportedOps & DO_MOVEABLE) &&
                                    (usDefaultOp == DO_MOVE)) )
                {
                    // The NULL as the last parameter means we don't care
                    // about the Format, just the Rendering Mechanism.

                    if( DrgVerifyRMF( pDragItem, "DRM_OS2FILE", NULL ) )
                        usDrop = DOR_DROP;
                    else
                    {
                        usDrop = DOR_NEVERDROP;
                        break;
                    }
                }
                else
                {
                   usDrop = DOR_NODROPOP;
                   break;
                }
            }
            else
                Msg( "dragOver DrgQueryDragitemPtr RC(%X)", HWNDERR(hwndFrame));
        }

    // If the user requested, via the Settings notebook, to use a specific
    // drop action and default operation, use it.

    if( dlgInfo.fUseDlgDragOvers )
    {
        usDrop      = dlgInfo.usDragOverDrop;
        usDefaultOp = dlgInfo.usDragOverDefOp;
    }

    // Free our handle to the shared memory if the source window is not in our
    // process.

    if( !DrgFreeDraginfo( pDragInfo ) &&
        PMERR_SOURCE_SAME_AS_TARGET != HWNDERR( hwndFrame ) )
        Msg( "dragOver DrgFreeDraginfo RC(%X)", HWNDERR( hwndFrame ) );

    return MRFROM2SHORT( usDrop, usDefaultOp );
}

/**********************************************************************/
/*--------------------------- dragLeave ------------------------------*/
/*                                                                    */
/*  PROCESS CN_DRAGLEAVE NOTIFY MESSAGE.                              */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the CNRDRAGINFO structure                       */
/*                                                                    */
/*  NOTES: The container sends the CN_DRAGLEAVE message to its owner  */
/*         when it gets a DM_DRAGLEAVE message. The container takes   */
/*         the info it gets on the DM_DRAGLEAVE message and combines  */
/*         it with other container-specific dragleave info into a     */
/*         CNRDRAGINFO structure, then passes a pointer to that       */
/*         structure to its owner in the CN_DRAGLEAVE message.        */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void dragLeave( HWND hwndFrame, PCNRDRAGINFO pcdi )
{
    PDRAGINFO pDragInfo = pcdi->pDragInfo;
    PINSTANCE pi = INSTDATA( hwndFrame );

    if( !pi )
    {
        Msg( "dragLeave cant get Inst data RC(%X)", HWNDERR( hwndFrame ) );
        return;
    }

    pi->fDragoverInProgress = FALSE;

    if( !DrgAccessDraginfo( pDragInfo ) )
    {
        Msg( "dragLeave DrgAccessDraginfo RC(%X)", HWNDERR( hwndFrame ) );
        return;
    }

    showDragInfo( hwndFrame, "DM_DRAGLEAVE", pDragInfo );

    if( !DrgFreeDraginfo( pDragInfo ) &&
        PMERR_SOURCE_SAME_AS_TARGET != HWNDERR( hwndFrame ) )
        Msg( "dragDrop DrgFreeDraginfo RC(%X)", HWNDERR( hwndFrame ) );
}

/**********************************************************************/
/*---------------------------- dragDrop ------------------------------*/
/*                                                                    */
/*  PROCESS CN_DROP NOTIFY MESSAGE.                                   */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the CNRDRAGINFO structure                       */
/*                                                                    */
/*  NOTES: The container sends the CN_DROP message to its owner when  */
/*         it gets a DM_DROP message. The container takes the info it */
/*         gets on the DM_DROP message and combines it with other     */
/*         container-specific dragover info into a CNRDRAGINFO        */
/*         structure, then passes a pointer to that structure to its  */
/*         owner in the CN_DROP message.                              */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void dragDrop( HWND hwndFrame, PCNRDRAGINFO pcdi )
{
    PDRAGINFO pDragInfo = pcdi->pDragInfo;
    PDRAGITEM pDragItem;
    HWND      hwndCnr = WinWindowFromID( hwndFrame, FID_CLIENT );
    CNRINFO   cnri;
    BOOL      fRecordsExisted = FALSE;
    int       i;
    PINSTANCE pi = INSTDATA( hwndFrame );

    if( !pi )
    {
        Msg( "dragDrop cant get Inst data RC(%X)", HWNDERR( hwndFrame ) );
        return;
    }

    if( !DrgAccessDraginfo( pDragInfo ) )
    {
        Msg( "dragDrop DrgAccessDraginfo RC(%X)", HWNDERR( hwndFrame ) );
        return;
    }

    showDragInfo( hwndFrame, "DM_DROP", pDragInfo );

    // Check if there are records in the container. If not, and we insert some
    // later, we want to change the title of the container.

    WinSendMsg( hwndCnr, CM_QUERYCNRINFO, MPFROMP( &cnri ),
                MPFROMLONG( sizeof cnri ) );
    if( cnri.cRecords )
        fRecordsExisted = TRUE;

    // Process each DragItem. First get a pointer to it, then call a function
    // that does the drop processing.

    for( i = 0; i < pDragInfo->cditem; i++ )
    {
        pDragItem = DrgQueryDragitemPtr( pDragInfo, i );

        if( pDragItem )
            ProcessDroppedItem( hwndFrame, pDragItem );
        else
            Msg( "dragDrop DrgQueryDragitemPtr RC(%X)", HWNDERR( hwndFrame ) );
    }

    // If records were added to an empty container, change the title to give
    // different instructions.

    if( !fRecordsExisted )
    {
        WinSendMsg( hwndCnr, CM_QUERYCNRINFO, MPFROMP( &cnri ),
                    MPFROMLONG( sizeof cnri ) );
        if( cnri.cRecords )
        {
            cnri.cb           = sizeof( CNRINFO );
            cnri.pszCnrTitle  = szDragCnrTitle;
            WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnri ),
                        MPFROMLONG( CMA_CNRTITLE ) );
        }
    }

    // If a drop happens, the DM_DRAGLEAVE doesn't get sent so we must turn off
    // that flag here.

    pi->fDragoverInProgress = FALSE;

    // It is the target's responsibility to delete the string resources. This
    // one API frees all the strings in all DRAGITEM structures.

    if( !DrgDeleteDraginfoStrHandles( pDragInfo ) )
        Msg( "dragDrop DrgDeleteDraginfoStrHandles RC(%X)", HWNDERR(hwndFrame));

    // Both the source and target must free the DRAGINFO structure.

    if( !DrgFreeDraginfo( pDragInfo ) &&
        PMERR_SOURCE_SAME_AS_TARGET != HWNDERR( hwndFrame ) )
        Msg( "dragDrop DrgFreeDraginfo RC(%X)", HWNDERR( hwndFrame ) );
}

/**********************************************************************/
/*------------------------ CountSelectedRecs -------------------------*/
/*                                                                    */
/*  COUNT THE NUMBER OF RECORDS THAT ARE CURRENTLY SELECTED.          */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the record that was under the pointer,          */
/*         address of BOOL - should we process selected records?      */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: number of records to process                             */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
int CountSelectedRecs( HWND hwndFrame, PCNRREC pCnrRecUnderMouse,
                       PBOOL pfUseSelectedRecs )
{
    int cRecs = 0;

    *pfUseSelectedRecs = FALSE;

    // If the record under the mouse is NULL, we must be over whitespace, in
    // which case we don't want to drag any records.

    if( pCnrRecUnderMouse )
    {
        PCNRREC pCnrRec = (PCNRREC) CMA_FIRST;

        // Count the records with 'selection' emphasis. These are the records
        // we want to drag, unless the container record under the mouse does
        // not have selection emphasis. If that is the case, we only want to
        // process that one.

        while( pCnrRec )
        {
            pCnrRec = (PCNRREC) WinSendDlgItemMsg( hwndFrame, FID_CLIENT,
                                                   CM_QUERYRECORDEMPHASIS,
                                                   MPFROMP( pCnrRec ),
                                                   MPFROMSHORT( CRA_SELECTED ));

            if( pCnrRec == (PCNRREC) -1 )
                Msg( "CountSelectedRecs..CM_QUERYRECORDEMPHASIS RC(%X)",
                     HWNDERR( hwndFrame ) );
            else if( pCnrRec )
            {
                if( pCnrRec == pCnrRecUnderMouse )
                    *pfUseSelectedRecs = TRUE;

                cRecs++;
            }
        }

        if( !(*pfUseSelectedRecs) )
            cRecs = 1;
    }

    return cRecs;
}

/**********************************************************************/
/*----------------------- SetSelectedDragItems -----------------------*/
/*                                                                    */
/*  FILL THE DRAGINFO STRUCT WITH DRAGITEM STRUCTS FOR SELECTED RECS. */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         count of selected records,                                 */
/*         pointer to allocated DRAGINFO struct,                      */
/*         pointer to allocated DRAGIMAGE array                       */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void SetSelectedDragItems( HWND hwndFrame, int cRecs, PDRAGINFO pDragInfo,
                           PDRAGIMAGE pDragImage )
{
    PCNRREC pCnrRec = (PCNRREC) CMA_FIRST;
    int     i;

    for( i = 0; i < cRecs; i++, pDragImage++ )
    {
        pCnrRec = (PCNRREC) WinSendDlgItemMsg( hwndFrame, FID_CLIENT,
                                               CM_QUERYRECORDEMPHASIS,
                                               MPFROMP( pCnrRec ),
                                               MPFROMSHORT( CRA_SELECTED ) );

        if( pCnrRec == (PCNRREC) -1 )
            Msg( "SetSelectedDragItems..CM_QUERYRECORDEMPHASIS RC(%X)",
                 HWNDERR( hwndFrame ) );
        else
            SetOneDragItem( hwndFrame, pCnrRec, pDragInfo, pDragImage, i );
    }
}

/**********************************************************************/
/*-------------------------- SetOneDragItem --------------------------*/
/*                                                                    */
/*  SET ONE DRAGITEM STRUCT INTO A DRAGINFO STRUCT.                   */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to CNRREC that contains current container record,  */
/*         pointer to allocated DRAGINFO struct,                      */
/*         pointer to allocated DRAGIMAGE array,                      */
/*         record offset into DRAGINFO struct to place DRAGITEM       */
/*                                                                    */
/*  NOTES: Fill in a DRAGITEM struct and 'set' it into the DRAGINFO   */
/*         shared memory. Also fill in a DRAGIMAGE structure so PM    */
/*         knows what image to use in representing this item.         */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void SetOneDragItem( HWND hwndFrame, PCNRREC pCnrRec, PDRAGINFO pDragInfo,
                     PDRAGIMAGE pDragImage, int iOffset )
{
    DRAGITEM DragItem;
    PCH      pchBackslash;
    char     chTemp;

    memset( &DragItem, 0, sizeof DragItem );

    // hwndDragItem is the window that will get the DM_RENDER and
    // DM_RENDERCOMPLETE messages.

    DragItem.hwndItem = hwndFrame;

    // ulItemID is used to store information that can be used at drop time. Here
    // we store the container record pointer, unless the user has overridden
    // the item ID thru the settings notebook.

    if( dlgInfo.fUseDlgItemID )
        DragItem.ulItemID = dlgInfo.ulItemID;
    else
        DragItem.ulItemID = (ULONG) pCnrRec;

    // hstrType identifies 'types' when it is necessary to differentiate. A
    // good example is if you are dragging file names (DRM_OS2FILE) and need to
    // pass the file type to the target (i.e. DRT_BITMAP would mean the file
    // contained a bitmap, DRT_TEXT is an ascii file, etc. ). We use the value
    // that was last entered in the settings notebook. It defaults to
    // DRT_UNKNOWN if nothing was entered. This means that the file type is not
    // known.

    DragItem.hstrType = DrgAddStrHandle( dlgInfo.szType );

    // We get the RMF from the global DLGINFO structure that can be modified
    // thru the settings notebook. The default hstrRMF is
    // "(DRM_OS2FILE,DRM_PRINT,DRM_DISCARD)x(DRF_TEXT)". This would equate to
    // allowing a 'file-based' drop as well as allowing drops on the printer
    // and shredder and using 'text'-based communication.

    DragItem.hstrRMF = DrgAddStrHandle( dlgInfo.fUseManualRMF ?
                                dlgInfo.szManualRMF : dlgInfo.szGeneratedRMF );

    // This will contain the directory of the file. Temporarily remove the
    // filename part of it so we can insert just the directory. After we create
    // the stringhande, replace that first character of the file name. If the
    // 'Override program values' checkbox was checked in the Settings notebook
    // we will use the values that the user keyed in. Otherwise we get the
    // values from the file that is being dragged.

    pchBackslash = strrchr( pCnrRec->szFullFileName, '\\' );
    if( pchBackslash )
    {
        chTemp = *(pchBackslash + 1);
        *(pchBackslash + 1) = 0;
    }

    if( dlgInfo.fUseDlgDragNames )
        DragItem.hstrContainerName = DrgAddStrHandle( dlgInfo.szContainerName);
    else
        DragItem.hstrContainerName = DrgAddStrHandle( pCnrRec->szFullFileName );

    if( pchBackslash )
        *(pchBackslash + 1) = chTemp;

    // This will contain the file name

    if( dlgInfo.fUseDlgDragNames )
        DragItem.hstrSourceName = DrgAddStrHandle( dlgInfo.szSourceName );
    else
        DragItem.hstrSourceName = DrgAddStrHandle( pCnrRec->szFileName );

    // Suggested target name is the same as the source name. We *could* ask the
    // target to call it something else but we don't need that functionality
    // in this program. If the user wants us to use the target name that was
    // specified in the Settings notebook, we do.

    if( dlgInfo.fUseDlgDragNames )
        DragItem.hstrTargetName = DrgAddStrHandle( dlgInfo.szTargetName );
    else
        DragItem.hstrTargetName = DragItem.hstrSourceName;

    // Get these values from the Settings notebook. The default is no control
    // flags. These flags are used to specify miscellaneous things about the
    // item being dragged.

    DragItem.fsControl = dlgInfo.fsControl;

    // Get these values from the Settings notebook. The default is to allow
    // the user to copy and move this file (DO_MOVEABLE | DO_COPYABLE).

    DragItem.fsSupportedOps = dlgInfo.fsSupportedOps;

    // Set the DRAGITEM struct into the memory allocated by
    // DrgAllocDraginfo()

    DrgSetDragitem( pDragInfo, &DragItem, sizeof DragItem, iOffset );

    // Fill in the DRAGIMAGE structure

    pDragImage->cb       = sizeof( DRAGIMAGE );
    pDragImage->hImage   = pCnrRec->mrc.hptrIcon; // DragImage under mouse
    pDragImage->fl       = DRG_ICON;              // hImage is an HPOINTER
    pDragImage->cxOffset = 5 * iOffset;           // Image offset from mouse ptr
    pDragImage->cyOffset = 5 * iOffset;           // Image offset from mouse ptr

    // Set source emphasis for this container record

    if( !WinSendDlgItemMsg( hwndFrame, FID_CLIENT, CM_SETRECORDEMPHASIS,
                        MPFROMP( pCnrRec ), MPFROM2SHORT( TRUE, CRA_SOURCE ) ) )
        Msg( "SetOneDragItem..CM_SETRECORDEMPHASIS RC(%X)",
             HWNDERR( hwndFrame ) );
}

/**********************************************************************/
/*----------------------- DetermineDefaultOp -------------------------*/
/*                                                                    */
/*  DETERMINE THE DEFAULT OPERATION BASED ON THE INFO IN DRAGINFO.    */
/*                                                                    */
/*  PARMS: pointer to the DRAGINFO structure                          */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: default operation. could be one of these                 */
/*             - DO_COPY       - 'Copy' is the default operation      */
/*             - DO_LINK       - 'Link' is the default operation      */
/*             - DO_MOVE       - 'Move' is the default operation      */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
USHORT DetermineDefaultOp( PDRAGINFO pDragInfo )
{
    USHORT usDefaultOp = 0;

    if( pDragInfo->usOperation == DO_DEFAULT )
    {
        PDRAGITEM pDragItem = DrgQueryDragitemPtr( pDragInfo, 0 );
        char      szSourcePath[ CCHMAXPATH ];

        *szSourcePath = 0;  // quicker than a memset for our purposes

        // If the current operation is DO_DEFAULT, we need to determine what
        // the default operation will be over our window. We rely on the first
        // DRAGITEM to make this decision - probably not a great idea, but the
        // toolkit sample does this too <g>...

        DrgQueryStrName( pDragItem->hstrContainerName, sizeof szSourcePath,
                         szSourcePath );

        // If the source and target are on the drive, make it a move.
        // Otherwise if it is on a floppy drive or it is RemovableMedia,
        // make it a copy.

        if( toupper( *szSourcePath ) == toupper( *szCurrentPath ) )
            usDefaultOp = DO_MOVE;
        else
            if( toupper( *szSourcePath == 'A' ) ||
                toupper( *szSourcePath == 'B' ) ||
                pDragItem->fsControl & DC_REMOVEABLEMEDIA )
                usDefaultOp = DO_COPY;
            else
                usDefaultOp = DO_MOVE;
    }
    else
        usDefaultOp = pDragInfo->usOperation;

    return usDefaultOp;
}

/**********************************************************************/
/*------------------------ ProcessDroppedItem ------------------------*/
/*                                                                    */
/*  PROCESS A DRAGITEM THAT HAS BEEN DROPPED ON US                    */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to DRAGITEM structure                              */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void ProcessDroppedItem( HWND hwndFrame, PDRAGITEM pDragItem )
{
    ULONG ulSuccess = DMFL_TARGETSUCCESSFUL;

    if( !DrgVerifyRMF( pDragItem, "DRM_OS2FILE", NULL ) )
    {
        // We can only handle the DRM_OS2FILE Rendering Mechanism in this
        // program. Normally we would have weeded out non-DRM_OS2FILE drags
        // on the DRAGOVER messages but we allow all kinds of drags in this
        // program so we can show in the debug window what would have been
        // dropped. So we allow the drop but issue a TargetFailed message on
        // the drop if a non-OS2FILE.

        ulSuccess = DMFL_TARGETFAIL;
    }
    else if( !pDragItem->hstrSourceName )
    {
        // One of the standards of the DRM_OS2FILE protocol is that if the
        // hstrSourceName (the file-name part of a fully-qualified file) is
        // NULL we need to ask the source to render. We don't handle rendering
        // in this program (check my DRGRENDR.EXE program) so we fail the drop.

        ulSuccess = DMFL_TARGETFAIL;
    }
    else
    {
        PCNRREC      pCnrRec;
        RECORDINSERT ri;
        HWND         hwndCnr = WinWindowFromID( hwndFrame, FID_CLIENT );

        memset( &ri, 0, sizeof ri );
        ri.cb                 = sizeof( RECORDINSERT );
        ri.pRecordOrder       = (PRECORDCORE) CMA_END;
        ri.pRecordParent      = (PRECORDCORE) NULL;
        ri.zOrder             = (USHORT) CMA_TOP;
        ri.cRecordsInsert     = 1;
        ri.fInvalidateRecord  = TRUE;

        pCnrRec = WinSendMsg( hwndCnr, CM_ALLOCRECORD, MPFROMLONG(EXTRA_BYTES),
                              MPFROMLONG( 1 ) );

        if( pCnrRec )
        {
            PSZ pszLoc;

            DrgQueryStrName( pDragItem->hstrContainerName,
                             sizeof pCnrRec->szFullFileName,
                             pCnrRec->szFullFileName );
            pszLoc = pCnrRec->szFullFileName + strlen( pCnrRec->szFullFileName);
            if( pCnrRec->szFullFileName[ strlen(pCnrRec->szFullFileName) - 1 ]
                                                                       != '\\' )
                pCnrRec->szFullFileName[ strlen(pCnrRec->szFileName) - 1 ]
                                                                       = '\\';
            DrgQueryStrName( pDragItem->hstrSourceName,
                             sizeof pCnrRec->szFullFileName -
                                   (pszLoc - pCnrRec->szFullFileName), pszLoc );

            pCnrRec->mrc.hptrIcon = WinLoadFileIcon( pCnrRec->szFullFileName,
                                                     FALSE );
            if( !pCnrRec->mrc.hptrIcon )
                pCnrRec->mrc.hptrIcon =
                    WinQuerySysPointer( HWND_DESKTOP, SPTR_QUESICON, FALSE );

            DrgQueryStrName( pDragItem->hstrSourceName,
                             sizeof pCnrRec->szFileName, pCnrRec->szFileName );

            pCnrRec->mrc.pszIcon  = (PSZ) &pCnrRec->szFileName;

            if( !WinSendMsg( hwndCnr, CM_INSERTRECORD,
                             MPFROMLONG( pCnrRec ), MPFROMP( &ri ) ) )
                Msg( "ProcessDroppedItem CM_INSERTRECORD RC(%X)",
                     HWNDERR( hwndFrame ) );
        }
        else
            Msg( "ProcessDroppedItem CM_ALLOCRECORD RC(%X)", HWNDERR(hwndFrame));
    }

    // Tell the source how the drop went for this item

    DrgSendTransferMsg( pDragItem->hwndItem, DM_ENDCONVERSATION,
                        MPFROMLONG( pDragItem->ulItemID ),
                        MPFROMLONG( ulSuccess ) );
}

/**********************************************************************/
/*--------------------------- PrintObject ----------------------------*/
/*                                                                    */
/*  PROCESS DM_PRINTOBJECT MESSAGE.                                   */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to DRAGITEM structure,                             */
/*         pointer to PRINTDEST structure                             */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: DRR_SOURCE or DRR_TARGET or DRR_ABORT                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT PrintObject( HWND hwndFrame, PDRAGITEM pDragItem, PPRINTDEST pPrintDest)
{
    HAB     hab = ANCHOR( hwndFrame );
    PCNRREC pCnrRec = (PCNRREC) pDragItem->ulItemID;
    HDC     hdc;

    showDragItem( hwndFrame, "DM_PRINTOBJECT", pDragItem, 0 );

    // If the user specified that they wanted to use their own item ID and
    // entered it in the settings notebook, we assume it is not a container
    // record pointer so we don't do anything.

    if( dlgInfo.fUseDlgItemID )
        return (MRESULT) DRR_SOURCE;

    // If the user, via the Settings notebook, told us not to do the printing
    // ourselves, just return with the reply that they want.

    if( dlgInfo.ulPrinterReply != DRR_SOURCE )
        return (MRESULT) dlgInfo.ulPrinterReply;

    // Get a printer device context using the information returned from PM
    // in the PRINTDEST structure

    hdc = DevOpenDC( hab, pPrintDest->lType, pPrintDest->pszToken,
                     pPrintDest->lCount, pPrintDest->pdopData, NULLHANDLE );

    if( pCnrRec && hdc )
    {
        LONG lBytes, lError;

        // If PM is telling us that we should let the user decide on job
        // properties, we accommodate it by putting up the job properties
        // dialog box.

        if( pPrintDest->fl & PD_JOB_PROPERTY )
            DevPostDeviceModes( hab,
                          ((PDEVOPENSTRUC) pPrintDest->pdopData)->pdriv,
                          ((PDEVOPENSTRUC) pPrintDest->pdopData)->pszDriverName,
                          pPrintDest->pszPrinter, NULL, DPDM_POSTJOBPROP );

        // Tell the spooler that we are starting a document.

        lError = DevEscape( hdc, DEVESC_STARTDOC, strlen( DOC_NAME ),
                            DOC_NAME, &lBytes, NULL );

        if( lError == DEV_OK )
        {
            char szPrintLine[ 100 ];

            sprintf( szPrintLine, "Printing %s\n", pCnrRec->szFullFileName );

            // Write the line to the printer

            lError = DevEscape( hdc, DEVESC_RAWDATA, strlen( szPrintLine ),
                                szPrintLine, &lBytes, NULL );

            if( lError == DEVESC_ERROR )
                Msg( "Bad DevEscape RAWDATA. RC(%X)", HWNDERR( hwndFrame ) );

            // Tell the spooler that we are done printing this document

            lError = DevEscape( hdc, DEVESC_ENDDOC, 0, NULL, &lBytes, NULL );

            if( lError == DEVESC_ERROR )
                Msg( "Bad DevEscape ENDDOC. RC(%X)", HWNDERR( hwndFrame ) );
        }
        else
            Msg( "Bad DevEscape STARTDOC for %s. RC(%X)", DOC_NAME,
                 HWNDERR( hwndFrame ) );
    }

    if( hdc )
        DevCloseDC( hdc );
    else
        Msg( "DevOpenDC failed. RC(%X)", HWNDERR( hwndFrame ) );

    // Tell PM that we are doing the printing. We could return DRR_TARGET
    // which would leave the printing burden on the printer object or DRR_ABORT
    // which would tell PM to abort the drop.

    showPrintReply( hwndFrame, "source DM_PRINTOBJECT reply: ", DRR_SOURCE );

    return (MRESULT) dlgInfo.ulPrinterReply;
}

/**********************************************************************/
/*-------------------------- DiscardObjects --------------------------*/
/*                                                                    */
/*  PROCESS DM_DISCARDOBJECT MESSAGE.                                 */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to DRAGINFO structure                              */
/*                                                                    */
/*  NOTE: We get a DM_DISCARDOBJECT message for each record being     */
/*        dropped. Since we get a DRAGINFO pointer the first time,    */
/*        we process all records the first time around. The rest of   */
/*        the times we go thru the motions but don't really do        */
/*        anything.                                                   */
/*                                                                    */
/*                                                                    */
/*  OUTPUT: DRR_SOURCE or DRR_TARGET or DRR_ABORT                     */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT DiscardObjects( HWND hwndFrame, PDRAGINFO pDragInfo )
{
    HWND      hwndCnr = WinWindowFromID( hwndFrame, FID_CLIENT );
    int       i, cRecsToDiscard;
    PDRAGITEM pDragItem;
    PCNRREC   *apCnrRec;

    showDragInfo( hwndFrame, "DM_DISCARDOBJECT", pDragInfo );

    // If the user specified that they wanted to use their own item ID and
    // entered it in the settings notebook, we assume it is not a container
    // record pointer so we don't do anything.

    if( dlgInfo.fUseDlgItemID )
        return (MRESULT) DRR_SOURCE;

    // If the user, via the Settings notebook, told us not to do the shredding
    // ourselves, just return with the reply that they want.

    if( dlgInfo.ulShredderReply != DRR_SOURCE )
        return (MRESULT) dlgInfo.ulShredderReply;

    // Allocate memory for an array of container record pointers. This array
    // will be used during the CM_REMOVERECORD message to remove all dropped
    // records in one shot

    apCnrRec = (PCNRREC *) _alloca( pDragInfo->cditem * sizeof( PCNRREC ) );

    memset( apCnrRec, 0, pDragInfo->cditem * sizeof( PCNRREC ) );

    cRecsToDiscard = pDragInfo->cditem;

    for( i = 0; i < pDragInfo->cditem; i++ )
    {
        pDragItem = DrgQueryDragitemPtr( pDragInfo, i );

        if( pDragItem )
            apCnrRec[ i ] = (PCNRREC) pDragItem->ulItemID;
        else
        {
            cRecsToDiscard--;
            Msg( "DiscardObjects DrgQueryDragitemPtr RC(%X)",
                 HWNDERR( hwndFrame ) );
        }
    }

    if( cRecsToDiscard )
    {
        // If CMA_FREE is used on CM_REMOVERECORD, the program traps on the
        // second DM_DISCARDOBJECT. So we can't blindly use CMA_FREE on the
        // CM_REMOVERECORD call. See comment below.

        int cRecsLeft = (int) WinSendMsg( hwndCnr, CM_REMOVERECORD,
                 MPFROMP( apCnrRec ),
                 MPFROM2SHORT( cRecsToDiscard, CMA_INVALIDATE ) );

        // -1 means invalid parameter. The problem is that we get as many
        // DM_DISCARDOBJECT messages as there are records dragged but the
        // first one has enough info to process all of them. The messages
        // after the first are irrelevant because we already did the
        // removing. Since subsequent DM_DISCARDOBJECT messages will cause
        // the above to fail (records were already moved the first time),
        // we need to not try and free the records here. If we do, we'll
        // trap because they were freed the first time.

        if( cRecsLeft != -1 )
            WinSendMsg( hwndCnr, CM_FREERECORD, MPFROMP( apCnrRec ),
                        MPFROMSHORT( cRecsToDiscard ) );
    }

    // Tell PM that we are doing the discarding. We could return DRR_TARGET
    // which would leave the discarding burden on the shredder or DRR_ABORT
    // which would tell PM to abort the drop.

    showPrintReply( hwndFrame, "source DM_DISCARDOBJECT reply: ", DRR_SOURCE );

    return (MRESULT) dlgInfo.ulShredderReply;
}

/**********************************************************************/
/*-------------------------- EndConversation -------------------------*/
/*                                                                    */
/*  FREE THE RESOURCES USED BY DRAG/DROP PROCESSING. ONLY DO THIS IF  */
/*  THIS IS THE LAST ITEM (there is one end-conversation message sent */
/*  to us for each item dropped).                                     */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to Container record that was dropped,              */
/*         success indicator                                          */
/*                                                                    */
/*  NOTES: This is a SOURCE window message that the target sends after*/
/*         it is done processing the DM_RENDERCOMPLETE message that   */
/*         the source sent it.                                        */
/*                                                                    */
/*  RETURNS: MRESULT value                                            */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EndConversation( HWND hwndFrame, PCNRREC pCnrRec, ULONG flSuccess )
{
    PINSTANCE pi = INSTDATA( hwndFrame );
    char      szPrefix[ 50 ];
    BOOL      fCnrRecOK = TRUE;

    if( !pi )
    {
        Msg( "EndConversation cant get Inst data RC(%X)", HWNDERR(hwndFrame) );
        return 0;
    }

    sprintf( szPrefix, "*** DM_ENDCONVERSATION for ulItemId %p: ", pCnrRec );
    showRenderReply( hwndFrame, szPrefix, flSuccess );

    // If the user specified that they wanted to use their own item ID and
    // entered it in the settings notebook, we assume it is not a container
    // record pointer so we don't do anything.

    if( dlgInfo.fUseDlgItemID )
        fCnrRecOK = FALSE;

    // If the drop was successful and it was a move operation, remove the
    // record from our container.

    if( (flSuccess & DMFL_TARGETSUCCESSFUL) && pCnrRec && fCnrRecOK )
        if( pi->pSavedDragInfo->usOperation == DO_MOVE )
            WinSendDlgItemMsg( hwndFrame, FID_CLIENT, CM_REMOVERECORD,
                               MPFROMP( &pCnrRec ),
                               MPFROM2SHORT( 1, CMA_INVALIDATE | CMA_FREE ) );

    // We need to keep a running total to know when all items in the drop have
    // been processed. When that happens, it is time to free the resources that
    // were allocated to the drag as a whole rather than to an indidvidual item.

    if( --pi->cDragItems == 0 )
    {
        // Free the shared memory we got access to using DrgAccessDragInfo. If
        // the source process is the same as the target process, we get the
        // PMERR_SOURCE_SAME_AS_TARGET message. It's ok to get that - it just
        // means that we don't need to free the structure because the target
        // process already freed it.

        if( !DrgFreeDraginfo( pi->pSavedDragInfo ) &&
            PMERR_SOURCE_SAME_AS_TARGET != HWNDERR( hwndFrame ) )
            Msg( "SourceCleanup DrgFreeDraginfo RC(%X)", HWNDERR( hwndFrame ) );

        // This is important because the NULL-ness of pSavedDragInfo lets the
        // dragInit function know that another drag is now possible.

        pi->pSavedDragInfo = NULL;
        pi->cDragItems     = 0;
    }

    // We need to know when the first DM_DRAGOVERNOTIFY message was hit so we
    // know how to display the message in the debug window. Here we turn that
    // flag off so that the DragoverNotify function knows when it gets the first
    // DM_DRAGOVERNOTIFY in a drag.

    pi->fGotFirstDragoverNotify = FALSE;

    return 0;
}

/**********************************************************************/
/*-------------------------- DragoverNotify --------------------------*/
/*                                                                    */
/*  PROCESS THE DM_DRAGOVERNOTIFY MESSAGE.                            */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the DRAGINFO structure,                         */
/*         MRESULT that the target window returned                    */
/*                                                                    */
/*  NOTES: This is a SOURCE window message that PM sends to the source*/
/*         after the target has processed a DM_DRAGOVER message. It   */
/*         allows the source to see what's happening at the target    */
/*         end.                                                       */
/*                                                                    */
/*  RETURNS: MRESULT value                                            */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void DragoverNotify( HWND hwndFrame, PDRAGINFO pDragInfo, MRESULT mr )
{
    PINSTANCE pi = INSTDATA( hwndFrame );

    if( !pi )
    {
        Msg( "DragoverNotify cant get Inst data RC(%X)", HWNDERR(hwndFrame) );
        return;
    }

    // We only want to display the full DRAGINFO structure in the debug window
    // if it is the first DM_DRAGOVERNOTIFY message while over our window. If we
    // display all of them the drag operation takes a real performance hit. If
    // the target's response (MRESULT) changes, we redisplay the DRAGINFO
    // structure. We clear the fDragInProgress flag in the DM_ENDCONVERSATION
    // message.

    if( pi->fGotFirstDragoverNotify )
        if( pi->mrDragoverNotify == mr )
            dbgInsert( pi->hwndDebug, " *** repeat DM_DRAGOVERNOTIFY" );
        else
        {
            pi->mrDragoverNotify = mr;
            showDragoverNotifyInfo( hwndFrame, pDragInfo, mr );
        }
    else
    {
        pi->mrDragoverNotify = mr;
        pi->fGotFirstDragoverNotify = TRUE;
        showDragoverNotifyInfo( hwndFrame, pDragInfo, mr );
    }

}

/**********************************************************************/
/*----------------------- RemoveSourceEmphasis -----------------------*/
/*                                                                    */
/*  REMOVE SOURCE EMPHASIS FROM THE DRAGGED RECORDS.                  */
/*                                                                    */
/*  PARMS: frame window handle                                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void RemoveSourceEmphasis( HWND hwndFrame )
{
    PCNRREC pCnrRec = (PCNRREC) CMA_FIRST;

    // For every record with source emphasis, remove it.

    while( pCnrRec )
    {
        pCnrRec = (PCNRREC) WinSendDlgItemMsg( hwndFrame, FID_CLIENT,
                                            CM_QUERYRECORDEMPHASIS,
                                            MPFROMP( pCnrRec ),
                                            MPFROMSHORT( CRA_SOURCE ) );

        if( pCnrRec == (PCNRREC) -1 )
            Msg( "RemoveSourceEmphasis..CM_QUERYRECORDEMPHASIS RC(%X)",
                 HWNDERR( hwndFrame ) );
        else if( pCnrRec )
            if( !WinSendDlgItemMsg( hwndFrame, FID_CLIENT,
                                    CM_SETRECORDEMPHASIS, MPFROMP( pCnrRec ),
                                    MPFROM2SHORT( FALSE, CRA_SOURCE ) ) )
                Msg( "RemoveSourceEmphasis..CM_SETRECORDEMPHASIS RC(%X)",
                     HWNDERR( hwndFrame ) );
    }
}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
