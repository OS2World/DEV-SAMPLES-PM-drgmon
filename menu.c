/*********************************************************************
 *                                                                   *
 * MODULE NAME :  menu.c                 AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-22-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGDROP' drag/drop sample program.                  *
 *                                                                   *
 *  This module takes care of creating and destroying the container  *
 *  context menu.                                                    *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 *                                                                   *
 * FUNCTIONS AVALABLE TO OTHER MODULES:                              *
 *                                                                   *
 *   menuCreate                                                      *
 *   menuEnd                                                         *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  07-22-93 - Program coded.                                        *
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

#define  INCL_WINERRORS
#define  INCL_WINFRAMEMGR
#define  INCL_WINMENUS
#define  INCL_WINPOINTERS
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

HWND hwndMenu;

/**********************************************************************/
/*--------------------------- menuCreate -----------------------------*/
/*                                                                    */
/*  CREATE AND POSITION THE CONTEXT MENU.                             */
/*                                                                    */
/*  PARMS: frame window handle                                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void menuCreate( HWND hwndFrame )
{
    if( !hwndMenu || !WinIsWindow( ANCHOR( hwndFrame ), hwndMenu ) )
        hwndMenu = WinLoadMenu( hwndFrame, 0, ID_CONTEXT_MENU );

    if( hwndMenu )
    {
        POINTL ptl;

        // Get the position of the mouse pointer

        if( WinQueryPointerPos( HWND_DESKTOP, &ptl ) )
        {
            // Convert the position of the mouse pointer to coordinates with
            // respect to the frame window.

            if( WinMapWindowPoints( HWND_DESKTOP, hwndFrame, &ptl, 1 ) )
            {
                if( WinPopupMenu( hwndFrame, hwndFrame, hwndMenu, ptl.x, ptl.y,
                                  0, PU_HCONSTRAIN | PU_VCONSTRAIN |
                                  PU_KEYBOARD | PU_MOUSEBUTTON1 |
                                  PU_MOUSEBUTTON2 | PU_NONE ) )
                {
                    // If you don't do this PM will assign a menu id of
                    // FID_MENU.

                    WinSetWindowUShort( hwndMenu, QWS_ID, ID_CONTEXT_MENU );
                }
                else
                    Msg( "menuCreate WinPopupMenu failed! RC(%X)",
                         HWNDERR( hwndFrame ) );
            }
            else
                Msg( "menuCreate WinMapWindowPoints failed! RC(%X)",
                     HWNDERR( hwndFrame ) );
        }
        else
            Msg( "menuCreate WinQueryPointerPos failed! RC(%X)",
                 HWNDERR( hwndFrame ) );
    }
    else
        Msg( "menuCreate could not load the context menu. RC(%X)",
             HWNDERR( hwndFrame ) );
}

/**********************************************************************/
/*----------------------------- menuEnd ------------------------------*/
/*                                                                    */
/*  DESTROY THE MENU ON A WM_MENUEND MESSAGE.                         */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         menu id                                                    */
/*         menu window handle                                         */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void menuEnd( HWND hwndFrame, ULONG idMenu, HWND hwndMenu )
{
    if( idMenu == ID_CONTEXT_MENU && hwndMenu )
    {
        // After the menu is gone the container no longer has the focus so we
        // have to reset it.

        WinSetFocus( HWND_DESKTOP, WinWindowFromID( hwndFrame, FID_CLIENT ) );
    }
}

/**********************************************************************/
/*--------------------------- menuCommand ----------------------------*/
/*                                                                    */
/*  PROCESS A WM_COMMAND MESSAGE GENERATED FROM THE MENU.             */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         command id                                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void menuCommand( HWND hwndFrame, ULONG idCommand )
{
    switch( idCommand )
    {
        case IDM_DRAGINFO_SETTINGS:
            bookSetup( NBPID_DRAGINFO );
            break;

        case IDM_RMF_SETTINGS:
            bookSetup( NBPID_RMF );
            break;

        case IDM_REPLY_SETTINGS:
            bookSetup( NBPID_REPLY );
            break;

        case IDM_MISC_SETTINGS:
            bookSetup( NBPID_MISC );
            break;

        case IDM_SURFACE_LISTBOX:
            WinSetActiveWindow( HWND_DESKTOP,
                                INSTDATA( hwndFrame )->hwndDebug );
            break;

        case IDM_EXIT:
            WinPostMsg( hwndFrame, WM_SYSCOMMAND, MPFROMSHORT( SC_CLOSE ),
                        MPFROM2SHORT( CMDSRC_ACCELERATOR, FALSE ) );
            break;
    }
}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
