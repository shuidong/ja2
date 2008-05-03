#include "Cursors.h"
#include "Font_Control.h"
#include "TileDat.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "RenderWorld.h"
#include "Soldier_Find.h"
#include "Structure.h"
#include "Animation_Control.h"
#include "Points.h"
#include "Overhead.h"
#include "Structure_Wrap.h"
#include "Tile_Animation.h"
#include "Interactive_Tiles.h"
#include "Handle_Doors.h"
#include "Sound_Control.h"
#include "Interface.h"
#include "Keys.h"
#include "Message.h"
#include "Text.h"
#include "Random.h"
#include "SkillCheck.h"
#include "Dialogue_Control.h"
#include "Render_Fun.h"
#include "Quests.h"
#include "StrategicMap.h"
#include "Map_Screen_Interface_Map.h"
#include "Soldier_Profile.h"
#include "Isometric_Utils.h"
#include "AI.h"
#include "Soldier_Macros.h"
#include "Debug.h"


static BOOLEAN HandleDoorsOpenClose(SOLDIERTYPE* pSoldier, INT16 sGridNo, STRUCTURE* pStructure, BOOLEAN fNoAnimations);


void HandleDoorChangeFromGridNo( SOLDIERTYPE *pSoldier, INT16 sGridNo, BOOLEAN fNoAnimations )
{
	STRUCTURE *			pStructure;
	DOOR_STATUS *		pDoorStatus;
	BOOLEAN					fDoorsAnimated = FALSE;

	pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );

	if ( pStructure == NULL )
	{
#ifdef JA2TESTVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"ERROR: Told to handle door that does not exist at %d.", sGridNo );
#endif
		return;
	}

	fDoorsAnimated = HandleDoorsOpenClose( pSoldier, sGridNo, pStructure, fNoAnimations );
	if ( SwapStructureForPartner( sGridNo, pStructure ) != NULL)
	{
		RecompileLocalMovementCosts( sGridNo );
	}


	// set door busy
	pDoorStatus = GetDoorStatus( sGridNo );
	if ( pDoorStatus == NULL )
	{
#ifdef JA2TESTVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"ERROR: Told to set door busy but can't get door status at %d!", sGridNo );
#endif
		return;
	}

	// ATE: Only do if animated.....
	if ( fDoorsAnimated )
	{
		pDoorStatus->ubFlags |= DOOR_BUSY;
	}
}

UINT16 GetAnimStateForInteraction( SOLDIERTYPE *pSoldier, BOOLEAN fDoor, UINT16 usAnimState )
{
	switch( usAnimState )
	{
		case OPEN_DOOR:

      if ( pSoldier->ubBodyType == CRIPPLECIV )
      {
        return( CRIPPLE_OPEN_DOOR );
      }
      else
      {
			  if ( fDoor )
			  {
				  if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND )
				  {
					  return( OPEN_DOOR_CROUCHED );
				  }
				  else
				  {
					  return( usAnimState );
				  }
			  }
			  else
			  {
				  if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND )
				  {
					  return( BEGIN_OPENSTRUCT_CROUCHED );
				  }
				  else
				  {
					  return( BEGIN_OPENSTRUCT );
				  }
			  }
      }

		case CLOSE_DOOR:

      if ( pSoldier->ubBodyType == CRIPPLECIV )
      {
        return( CRIPPLE_CLOSE_DOOR );
      }
      else
      {
			  if ( fDoor )
			  {
				  if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND )
				  {
					  return( CLOSE_DOOR_CROUCHED );
				  }
				  else
				  {
					  return( usAnimState );
				  }
			  }
			  else
			  {
				  if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND )
				  {
					  return( OPEN_STRUCT_CROUCHED );
				  }
				  else
				  {
					  return( OPEN_STRUCT );
				  }
			  }
      }

		case END_OPEN_DOOR:

      if ( pSoldier->ubBodyType == CRIPPLECIV )
      {
        return( CRIPPLE_END_OPEN_DOOR );
      }
      else
      {
			  if ( fDoor )
			  {
				  if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND )
				  {
					  return( END_OPEN_DOOR_CROUCHED );
				  }
				  else
				  {
					  return( usAnimState );
				  }
			  }
			  else
			  {
				  if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND )
				  {
					  return( END_OPENSTRUCT_CROUCHED );
				  }
				  else
				  {
					  return( END_OPENSTRUCT );
				  }
			  }
      }

		case END_OPEN_LOCKED_DOOR:

      if ( pSoldier->ubBodyType == CRIPPLECIV )
      {
        return( CRIPPLE_END_OPEN_LOCKED_DOOR );
      }
      else
      {
			  if ( fDoor )
			  {
				  if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND )
				  {
					  return( END_OPEN_LOCKED_DOOR_CROUCHED );
				  }
				  else
				  {
					  return( END_OPEN_LOCKED_DOOR );
				  }
			  }
			  else
			  {
				  if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND )
				  {
					  return( END_OPENSTRUCT_LOCKED_CROUCHED );
				  }
				  else
				  {
					  return( END_OPENSTRUCT_LOCKED );
				  }
			  }
      }

		case PICK_LOCK:

			if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_STAND )
			{
        return( LOCKPICK_CROUCHED );
      }
      else
      {
        return( PICK_LOCK );
      }

		default:
			// should never happen!
			Assert( FALSE );
			return( usAnimState );
	}
}


void InteractWithClosedDoor( SOLDIERTYPE *pSoldier, UINT8 ubHandleCode )
{
	pSoldier->ubDoorHandleCode = ubHandleCode;

	UINT16 state;
	switch( ubHandleCode )
	{
		case HANDLE_DOOR_OPEN:
		case HANDLE_DOOR_UNLOCK:
		case HANDLE_DOOR_EXAMINE:
		case HANDLE_DOOR_EXPLODE:
		case HANDLE_DOOR_LOCK:
		case HANDLE_DOOR_UNTRAP:
		case HANDLE_DOOR_CROWBAR:
			state = GetAnimStateForInteraction(pSoldier, TRUE, OPEN_DOOR);
			break;

		case HANDLE_DOOR_FORCE:
			state = KICK_DOOR;
			break;

		case HANDLE_DOOR_LOCKPICK:
			state = GetAnimStateForInteraction(pSoldier, TRUE, PICK_LOCK);
			break;

		default: return;
	}
	ChangeSoldierState(pSoldier, state, 0 , FALSE);
}


static BOOLEAN DoTrapCheckOnStartingMenu(SOLDIERTYPE* pSoldier, DOOR* pDoor)
{
	INT8	bDetectLevel;

	if ( pDoor && pDoor->fLocked && pDoor->ubTrapID != NO_TRAP && pDoor->bPerceivedTrapped == DOOR_PERCEIVED_UNKNOWN )
	{
		// check for noticing the trap
		bDetectLevel = CalcTrapDetectLevel( pSoldier, FALSE );
		if (bDetectLevel >= pDoor->ubTrapLevel)
		{
			// say quote, update status
			TacticalCharacterDialogue( pSoldier, QUOTE_BOOBYTRAP_ITEM );
			UpdateDoorPerceivedValue( pDoor );

      return( TRUE );
		}
	}

  return( FALSE );
}

void InteractWithOpenableStruct( SOLDIERTYPE *pSoldier, STRUCTURE *pStructure, UINT8 ubDirection, BOOLEAN fDoor )
{
	STRUCTURE *			pBaseStructure;
	DOOR_STATUS *		pDoorStatus;
  BOOLEAN         fTrapsFound = FALSE;

	pBaseStructure = FindBaseStructure( pStructure );

	if ( fDoor )
	{
		// get door status, if busy then just return!
		pDoorStatus = GetDoorStatus( pBaseStructure->sGridNo );
		if ( pDoorStatus && (pDoorStatus->ubFlags & DOOR_BUSY) )
		{
			// Send this guy into stationary stance....
			EVENT_StopMerc( pSoldier, pSoldier->sGridNo, pSoldier->bDirection );

			if ( pSoldier->bTeam == gbPlayerNum )
			{
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ DOOR_IS_BUSY ] );
			}
      else
      {
    		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Trying to open door and door is busy: %d", pSoldier->ubID ) );
      }
			return;
		}
	}

	EVENT_SetSoldierDesiredDirectionForward(pSoldier, ubDirection);

	// Is the door opened?
	if ( pStructure->fFlags & STRUCTURE_OPEN )
	{
		if (IsOnOurTeam(pSoldier) && !(pStructure->fFlags & STRUCTURE_SWITCH))
		{
			// Bring up menu to decide what to do....
			SoldierGotoStationaryStance( pSoldier );

			DOOR* const pDoor = FindDoorInfoAtGridNo(pBaseStructure->sGridNo);
			if ( pDoor )
			{
				if ( DoTrapCheckOnStartingMenu( pSoldier, pDoor ) )
        {
          fTrapsFound = TRUE;
        }
			}

			// Pull Up Menu
      if ( !fTrapsFound )
      {
				InitDoorOpenMenu(pSoldier, TRUE);
      }
		}
		else
		{
			// Easily close door....
			ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, CLOSE_DOOR ), 0, FALSE );
		}
	}
	else
	{
		if (IsOnOurTeam(pSoldier))
		{
			DOOR* const pDoor = FindDoorInfoAtGridNo(pBaseStructure->sGridNo);
			if (pDoor != NULL && pDoor->fLocked) // Bring up the menu, only if it has a lock!
			{
				// Bring up menu to decide what to do....
				SoldierGotoStationaryStance(pSoldier);

				if (DoTrapCheckOnStartingMenu(pSoldier, pDoor))
				{
					fTrapsFound = TRUE;
				}

				// Pull Up Menu
				if (!fTrapsFound)
				{
					InitDoorOpenMenu(pSoldier, FALSE);
				}
				else
				{
					UnSetUIBusy(pSoldier);
				}
				return;
			}
		}

		pSoldier->ubDoorHandleCode = HANDLE_DOOR_OPEN;
		ChangeSoldierState(pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, OPEN_DOOR), 0, FALSE);
	}
}


static void ProcessImplicationsOfPCMessingWithDoor(SOLDIERTYPE* pSoldier)
{
	UINT8						ubRoom;
	// if player is hacking at a door in the brothel and a kingpin guy can see him
	if ( (InARoom( pSoldier->sGridNo, &ubRoom ) && IN_BROTHEL( ubRoom )) || (gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_D && gbWorldSectorZ == 0 && (pSoldier->sGridNo == 11010 || pSoldier->sGridNo == 11177 || pSoldier->sGridNo == 11176 ) ) )
	{
		// see if a kingpin goon can see us
		FOR_ALL_IN_TEAM(pGoon, CIV_TEAM)
		{
			if (pGoon->ubCivilianGroup == KINGPIN_CIV_GROUP &&
					pGoon->bInSector &&
					pGoon->bLife >= OKLIFE &&
					pGoon->bOppList[pSoldier->ubID] == SEEN_CURRENTLY)
			{
				MakeCivHostile( pGoon, 2 );
				if ( ! (gTacticalStatus.uiFlags & INCOMBAT) )
				{
					EnterCombatMode( pGoon->bTeam );
				}
			}
		}
	}

	if ( gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y )
	{
		SOLDIERTYPE* const pGoon = FindSoldierByProfileID(WARDEN);
		if ( pGoon && pGoon->bAlertStatus < STATUS_RED && PythSpacesAway( pSoldier->sGridNo, pGoon->sGridNo ) <= 5 )
		{
			// alert her if she hasn't been alerted
			pGoon->bAlertStatus = STATUS_RED;
			CheckForChangingOrders( pGoon );
			CancelAIAction(pGoon);
		}
	}
}



BOOLEAN HandleOpenableStruct( SOLDIERTYPE *pSoldier, INT16 sGridNo, STRUCTURE *pStructure )
{
	BOOLEAN fHandleDoor = FALSE;
	INT16		sAPCost = 0, sBPCost = 0;
	DOOR		*pDoor;
	BOOLEAN	fTrapFound = FALSE;
	BOOLEAN	fDoAction = TRUE;
	BOOLEAN	fDoor     = FALSE;

	// Are we a door?
	if (pStructure->fFlags & STRUCTURE_ANYDOOR)
	{
		fDoor = TRUE;
	}

	// Calculate basic points...

	// We'll add any aps for things like lockpicking, booting, etc

	// If we are already open....no need for lockpick checks, etc
	if ( pStructure->fFlags & STRUCTURE_OPEN )
	{
		// Set costs for these
		sAPCost = AP_OPEN_DOOR;
		sBPCost = BP_OPEN_DOOR;

		fHandleDoor = TRUE;
	}
	else
	{
		if (IsOnOurTeam(pSoldier))
		{
			// Find locked door here....
			pDoor = FindDoorInfoAtGridNo( sGridNo );

			// Alrighty, first check for traps ( unless we are examining.... )
			if ( pSoldier->ubDoorHandleCode != HANDLE_DOOR_EXAMINE && pSoldier->ubDoorHandleCode != HANDLE_DOOR_UNTRAP && pSoldier->ubDoorHandleCode != HANDLE_DOOR_UNLOCK )
			{
				if ( pDoor != NULL )
				{
					// Do we have a trap? NB if door is unlocked disable all traps
					if (pDoor->fLocked && pDoor->ubTrapID != NO_TRAP)
					{

						fTrapFound = TRUE;

						// Set costs for these
						// Set AP costs to that of opening a door
						sAPCost = AP_OPEN_DOOR;
						sBPCost = BP_OPEN_DOOR;

						ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );

						// Did we inadvertently set it off?
						if ( HasDoorTrapGoneOff( pSoldier, pDoor ) )
						{
							// Kaboom
							// Code to handle trap here...
							HandleDoorTrap( pSoldier, pDoor );
							if (DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_STOPS_ACTION)
							{
								// trap stops person from opening door!
								fDoAction	 = FALSE;
							}
							if (!( DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_RECURRING ) )
							{
								// trap only happens once
								pDoor->ubTrapLevel = 0;
								pDoor->ubTrapID = NO_TRAP;
							}
							UpdateDoorPerceivedValue( pDoor );
						}
						else
						{
							// If we didn't set it off then we must have noticed it or know about it already

							// do we know it's trapped?
							if ( pDoor->bPerceivedTrapped == DOOR_PERCEIVED_UNKNOWN )
							{
								switch( pDoor->ubTrapID )
								{
									case BROTHEL_SIREN:
										ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESCRIPTION_STR ], pDoorTrapStrings[ SIREN ] );
										break;
									case SUPER_ELECTRIC:
										ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESCRIPTION_STR ], pDoorTrapStrings[ ELECTRIC ] );
										break;
									default:
										ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESCRIPTION_STR ], pDoorTrapStrings[ pDoor->ubTrapID ] );
										break;
								}

								// Stop action this time....
								fDoAction	 = FALSE;

								// report!
								TacticalCharacterDialogue( pSoldier, QUOTE_BOOBYTRAP_ITEM );
							}
							else
							{
								// Set it off!
								HandleDoorTrap( pSoldier, pDoor );
								if (DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_STOPS_ACTION)
								{
									// trap stops person from opening door!
									fDoAction	 = FALSE;
								}
								if (!( DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_RECURRING ) )
								{
									// trap only happens once
									pDoor->ubTrapLevel = 0;
									pDoor->ubTrapID = NO_TRAP;
								}
							}
							UpdateDoorPerceivedValue( pDoor );

						}
					}
				}
			}

			if ( fDoAction )
			{
				// OK, switch based on how we are going to open door....
				switch( pSoldier->ubDoorHandleCode )
				{
					case HANDLE_DOOR_OPEN:

						// If we have no lock on door...
						if ( pDoor == NULL )
						{
							// Set costs for these
							sAPCost = AP_OPEN_DOOR;
							sBPCost = BP_OPEN_DOOR;

							// Open if it's not locked....
							ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
							fHandleDoor = TRUE;
						}
						else
						{
							if ( pDoor->fLocked )
							{
								// it's locked....
								ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_LOCKED_DOOR ), 0, FALSE );

								// Do we have a quote for locked stuff?
								// Now just show on message bar
								if ( !AM_AN_EPC( pSoldier ) )
								{
									DoMercBattleSound( pSoldier, BATTLE_SOUND_LOCKED );
								}
								else
								{
									ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_HAS_BEEN_LOCKED_STR ] );
								}
							}
							else
							{
								ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
								fHandleDoor = TRUE;
							}
							UpdateDoorPerceivedValue( pDoor );
						}
						break;

					case HANDLE_DOOR_FORCE:

						// Set costs for these
						sAPCost = AP_BOOT_DOOR;
						sBPCost = BP_BOOT_DOOR;

						// OK, using force, if we have no lock, just open the door!
						if ( pDoor == NULL )
						{
							ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
							fHandleDoor = TRUE;

							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							// Attempt to force door
							if ( AttemptToSmashDoor( pSoldier, pDoor ) )
							{
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESTROYED_STR ] );
								// DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
								fHandleDoor = TRUE;
							}
							else
							{
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_NOT_DESTROYED_STR ] );
								UpdateDoorPerceivedValue( pDoor );
							}
							ProcessImplicationsOfPCMessingWithDoor( pSoldier );
						}
						break;


					case HANDLE_DOOR_CROWBAR:

						// Set costs for these
						sAPCost = AP_USE_CROWBAR;
						sBPCost = BP_USE_CROWBAR;

						// OK, using force, if we have no lock, just open the door!
						if ( pDoor == NULL )
						{
							ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
							fHandleDoor = TRUE;

							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							// Attempt to force door
							if ( AttemptToCrowbarLock( pSoldier, pDoor ) )
							{
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESTROYED_STR ] );
								//DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
								fHandleDoor = TRUE;
							}
							else
							{
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_NOT_DESTROYED_STR ] );
								UpdateDoorPerceivedValue( pDoor );
							}

							ProcessImplicationsOfPCMessingWithDoor( pSoldier );
						}
						break;

					case HANDLE_DOOR_EXPLODE:

						// Set costs for these
						sAPCost = AP_EXPLODE_DOOR;
						sBPCost = BP_EXPLODE_DOOR;

						if ( pDoor == NULL )
						{
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							// Attempt to force door
							if ( AttemptToBlowUpLock( pSoldier, pDoor ) )
							{
								//DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESTROYED_STR ] );
								fHandleDoor = TRUE;
							}
							else
							{
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_NOT_DESTROYED_STR ] );
								UpdateDoorPerceivedValue( pDoor );
							}
							ProcessImplicationsOfPCMessingWithDoor( pSoldier );
						}
						break;

					case HANDLE_DOOR_LOCKPICK:

						// Set costs for these
						sAPCost = AP_PICKLOCK;
						sBPCost = BP_PICKLOCK;

						// Attempt to pick lock
						if ( pDoor == NULL )
						{
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							if ( AttemptToPickLock( pSoldier, pDoor ) )
							{
								DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_HAS_BEEN_PICKED_STR ] );
								fHandleDoor = TRUE;
							}
							else
							{
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_HAS_NOT_BEEN_PICKED_STR ] );
							}
							ProcessImplicationsOfPCMessingWithDoor( pSoldier );
						}
						break;


					case HANDLE_DOOR_EXAMINE:

						// Set costs for these
						sAPCost = AP_EXAMINE_DOOR;
						sBPCost = BP_EXAMINE_DOOR;

						// Attempt to examine door
						// Whatever the result, end the open animation
						ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );

						if ( pDoor == NULL )
						{
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							if ( ExamineDoorForTraps( pSoldier, pDoor ) )
							{
								// We have a trap. Use door pointer to determine what type, etc
								TacticalCharacterDialogue( pSoldier, QUOTE_BOOBYTRAP_ITEM );
								switch( pDoor->ubTrapID )
								{
									case BROTHEL_SIREN:
										ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESCRIPTION_STR ], pDoorTrapStrings[ SIREN ] );
										break;
									case SUPER_ELECTRIC:
										ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESCRIPTION_STR ], pDoorTrapStrings[ ELECTRIC ] );
										break;
									default:
										ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESCRIPTION_STR ], pDoorTrapStrings[ pDoor->ubTrapID ] );
										break;
								}

								UpdateDoorPerceivedValue( pDoor );
							}
							else
							{
								ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_UNTRAPPED_STR ] );
							}
						}
						break;


					case HANDLE_DOOR_UNLOCK:

						// Set costs for these
						sAPCost = AP_UNLOCK_DOOR;
						sBPCost = BP_UNLOCK_DOOR;

						// OK, if we have no lock, show that!
						if ( pDoor == NULL )
						{
							// Open if it's not locked....
							//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
							ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
							break;
						}
						else
						{

							// it's locked....
							// Attempt to unlock....
							if ( AttemptToUnlockDoor( pSoldier, pDoor ) )
							{
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_HAS_BEEN_UNLOCKED_STR ] );
								//DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );

								ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
								UpdateDoorPerceivedValue( pDoor );

								fHandleDoor = TRUE;
							}
							else
							{
								ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_LOCKED_DOOR ), 0, FALSE );
								// Do we have a quote for locked stuff?
								// Now just show on message bar
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_NOT_PROPER_KEY_STR ], pSoldier->name );

								// OK PLay damn battle sound
								if ( Random( 2 ) )
								{
									DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
								}
							}
						}
						break;


					case HANDLE_DOOR_UNTRAP:

						// Set costs for these
						sAPCost = AP_UNTRAP_DOOR;
						sBPCost = BP_UNTRAP_DOOR;

						// OK, if we have no lock, show that!
						if ( pDoor == NULL )
						{
							// Open if it's not locked....
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
							ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
							break;
						}
						else
						{
							// Do we have a trap?
							if (pDoor->ubTrapID != NO_TRAP)
							{
								fTrapFound = TRUE;
							}

							if ( fTrapFound )
							{
								if ( AttemptToUntrapDoor( pSoldier, pDoor ) )
								{
									//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_HAS_BEEN_UNTRAPPED_STR ] );
									DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
									ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
									UpdateDoorPerceivedValue( pDoor );
									//fHandleDoor = TRUE;
								}
								else
								{
									ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_LOCKED_DOOR ), 0, FALSE );
									// Now just show on message bar
									HandleDoorTrap( pSoldier, pDoor );

									if (!( DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_RECURRING ) )
									{
										// trap only happens once
										pDoor->ubTrapLevel = 0;
										pDoor->ubTrapID = NO_TRAP;
									}

									// Update perceived lock value
									UpdateDoorPerceivedValue( pDoor );
								}
							}
							else
							{
								ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_IS_NOT_TRAPPED_STR ] );
							}
						}
						break;


					case HANDLE_DOOR_LOCK:

						// Set costs for these
						sAPCost = AP_LOCK_DOOR;
						sBPCost = BP_LOCK_DOOR;

						// OK, if we have no lock, show that!
						if ( pDoor == NULL )
						{
							// Open if it's not locked....
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
							ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
							break;
						}
						else
						{

							// it's locked....
							// Attempt to unlock....
							if ( AttemptToLockDoor( pSoldier, pDoor ) )
							{
								ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_HAS_BEEN_LOCKED_STR ] );
								ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
								UpdateDoorPerceivedValue( pDoor );
							}
							else
							{
								ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_LOCKED_DOOR ), 0, FALSE );
								// Do we have a quote for locked stuff?
								// Now just show on message bar
								ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_NOT_PROPER_KEY_STR ], pSoldier->name );

								// Update perceived lock value
								UpdateDoorPerceivedValue( pDoor );
							}
						}
						break;
				}
			}
		}
		else
		{
			// Set costs for these
			sAPCost = AP_OPEN_DOOR;
			sBPCost = BP_OPEN_DOOR;

			// Open if it's not locked....
			ChangeSoldierState( pSoldier, GetAnimStateForInteraction( pSoldier, fDoor, END_OPEN_DOOR ), 0, FALSE );
			fHandleDoor = TRUE;
		}
	}

	if ( fHandleDoor )
	{
		if ( fDoor )
		{
			HandleDoorChangeFromGridNo( pSoldier, sGridNo, FALSE );
		}
		else
		{
			HandleStructChangeFromGridNo( pSoldier, sGridNo );
		}
	}

	// Deduct points!
	//if ( fDoor )
	{
		DeductPoints( pSoldier, sAPCost, sBPCost );
	}

	return( fHandleDoor );
}


static BOOLEAN HandleDoorsOpenClose(SOLDIERTYPE* pSoldier, INT16 sGridNo, STRUCTURE* pStructure, BOOLEAN fNoAnimations)
{
	LEVELNODE	* pShadowNode;
	LEVELNODE * pNode;
	INT32				cnt;
	BOOLEAN					fOpenedGraphic = FALSE;
	BOOLEAN					fDoAnimation = TRUE;
	STRUCTURE *	pBaseStructure;
	UINT32			uiSoundID;

	pBaseStructure = FindBaseStructure( pStructure );
	if (!pBaseStructure)
	{
		return( FALSE );
	}

	pNode = FindLevelNodeBasedOnStructure( pBaseStructure->sGridNo, pBaseStructure );
	if (!pNode)
	{
		return( FALSE );
	}

	// ATE: if we are about to swap, but have an animation playing here..... stop the animation....
	if ( ( pNode->uiFlags & LEVELNODE_ANIMATION ) )
	{
		if ( pNode->pAniTile != NULL )
		{
			if ( pNode->pAniTile->uiFlags & ANITILE_DOOR )
			{
				// ATE: No two doors can exist ( there can be only one )
				// Update value.. ie: prematurely end door animation
				// Update current frame...

				if ( pNode->pAniTile->uiFlags & ANITILE_FORWARD )
				{
					pNode->sCurrentFrame = pNode->pAniTile->sStartFrame + pNode->pAniTile->usNumFrames;
				}

				if ( pNode->pAniTile->uiFlags & ANITILE_BACKWARD )
				{
					pNode->sCurrentFrame = pNode->pAniTile->sStartFrame - pNode->pAniTile->usNumFrames;
				}

				pNode->sCurrentFrame = pNode->pAniTile->usNumFrames - 1;

				// Delete...
				DeleteAniTile( pNode->pAniTile );

				pNode->uiFlags &= ~( LEVELNODE_LASTDYNAMIC | LEVELNODE_UPDATESAVEBUFFERONCE );

				if ( GridNoOnScreen( pBaseStructure->sGridNo ) )
				{
					SetRenderFlags(RENDER_FLAG_FULL);
				}
			}
		}
	}


	pShadowNode = gpWorldLevelData[ sGridNo ].pShadowHead;


	// Check the graphic which is down!
	// Check for Open Door!
	cnt = 0;
	while( gOpenDoorList[ cnt ] != -1 )
	{
		// IF WE ARE A SHADOW TYPE
		if ( pNode->usIndex == gOpenDoorList[ cnt ] )
		{
			fOpenedGraphic = TRUE;
			break;
		}
		cnt++;
	}

	if ( !(pStructure->fFlags & STRUCTURE_OPEN) )
	{
		//ATE, the last parameter is the perceived value, I dont know what it is so could you please add the value?
		//ModifyDoorStatus( INT16 sGridNo, BOOLEAN fOpen, BOOLEAN fPercievedOpen )
		ModifyDoorStatus(sGridNo, TRUE, DONTSETDOORSTATUS);

		if ( gWorldSectorX == 13 && gWorldSectorY == MAP_ROW_I )
		{
			DoPOWPathChecks();
		}

		if (pSoldier )
		{
			// OK, Are we a player merc or AI?
			if ( pSoldier->bTeam != gbPlayerNum )
			{
				// If an AI guy... do LOS check first....
				// If guy is visible... OR fading...
				if (pSoldier->bVisible == -1 && !AllMercsLookForDoor(sGridNo) && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS))
				{
					fDoAnimation = FALSE;
				}
			}
		}
		else
		{
			// door opening by action item... just do a LOS check
			if (!AllMercsLookForDoor(sGridNo))
			{
				fDoAnimation = FALSE;
			}
		}

		if ( fNoAnimations )
		{
			fDoAnimation = FALSE;
		}

		if ( fDoAnimation  )
		{
			// Update perceived value
			ModifyDoorStatus( sGridNo, DONTSETDOORSTATUS, TRUE );

			ANITILE_PARAMS AniParams;
			memset(&AniParams, 0, sizeof(AniParams));
			AniParams.uiFlags         = ANITILE_DOOR | ANITILE_EXISTINGTILE | (fOpenedGraphic ? ANITILE_FORWARD : ANITILE_BACKWARD);
			AniParams.ubLevelID       = ANI_STRUCT_LEVEL;
			AniParams.sStartFrame     = pNode->sCurrentFrame;
			AniParams.sDelay          = INTTILE_DOOR_OPENSPEED;
			AniParams.usTileIndex     = pNode->usIndex;
			AniParams.sGridNo         = sGridNo;
			AniParams.pGivenLevelNode = pNode;
			CreateAnimationTile(&AniParams);
		}

		// SHADOW STUFF HERE
		//if ( pShadowNode != NULL )
		//{
		//	pShadowNode->uiFlags |= LEVELNODE_ANIMATION;
		//	pShadowNode->uiFlags |= LEVELNODE_ANIMATION_PLAYONCE;
		//	pShadowNode->uiFlags |= LEVELNODE_ANIMATION_FORWARD;
		//	if ( pShadowNode->uiFlags & LEVELNODE_ANIMATION_BACKWARD )
		//		pShadowNode->uiFlags ^= LEVELNODE_ANIMATION_BACKWARD;
		//	pShadowNode->sDelay		= INTTILE_DOOR_OPENSPEED;
		//}

		if ( fDoAnimation && pSoldier && pSoldier->ubDoorOpeningNoise)
		{
			// ATE; Default to normal door...
			uiSoundID = ( DROPEN_1 + Random( 3 ) );

			// OK, check if this door is sliding and is multi-tiled...
			if ( pStructure->fFlags & STRUCTURE_SLIDINGDOOR )
			{
				// Get database value...
				if ( pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1 )
				{
					// change sound ID
					uiSoundID = GARAGE_DOOR_OPEN;
				}
       	else if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_CLOTH )
	  		{
		  		// change sound ID
			  	uiSoundID = CURTAINS_OPEN;
			  }
      }
      else if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_LIGHT_METAL ||
           pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_THICKER_METAL ||
           pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_HEAVY_METAL )
			{
				// change sound ID
				uiSoundID = METAL_DOOR_OPEN;
			}

			// OK, We must know what sound to play, for now use same sound for all doors...
			PlayLocationJA2Sample(sGridNo, uiSoundID, MIDVOLUME, 1);
		}

	}
	else
	{
		//ATE, the last parameter is the perceived value, I dont know what it is so could you please add the value?
		//ModifyDoorStatus( INT16 sGridNo, BOOLEAN fOpen, BOOLEAN fInitiallyPercieveOpen )
		ModifyDoorStatus(sGridNo, FALSE, DONTSETDOORSTATUS);

		if (pSoldier )
		{
			// OK, Are we a player merc or AI?
			if ( pSoldier->bTeam != gbPlayerNum )
			{
				// If an AI guy... do LOS check first....
				// If guy is visible... OR fading...
				if (pSoldier->bVisible == -1 && !AllMercsLookForDoor(sGridNo) && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS))
				{
					fDoAnimation = FALSE;
				}
			}
		}
		else
		{
			// door opening by action item... just do a LOS check
			if (!AllMercsLookForDoor(sGridNo))
			{
				fDoAnimation = FALSE;
			}
		}

		if ( fNoAnimations )
		{
			fDoAnimation = FALSE;
		}

		if ( fDoAnimation )
		{
			// Update perceived value
			ModifyDoorStatus( sGridNo, DONTSETDOORSTATUS, FALSE );

			// ATE; Default to normal door...
			uiSoundID = ( DRCLOSE_1 + Random( 2 ) );

			// OK, check if this door is sliding and is multi-tiled...
			if ( pStructure->fFlags & STRUCTURE_SLIDINGDOOR )
			{
				// Get database value...
				if ( pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1 )
				{
					// change sound ID
					uiSoundID = GARAGE_DOOR_CLOSE;
				}
        else if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_CLOTH )
			  {
				  // change sound ID
				  uiSoundID = CURTAINS_CLOSE;
			  }
      }
		  else if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_LIGHT_METAL ||
           pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_THICKER_METAL ||
           pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_HEAVY_METAL )
			{
				// change sound ID
				uiSoundID = METAL_DOOR_CLOSE;
			}

			ANITILE_PARAMS AniParams;
			memset(&AniParams, 0, sizeof(AniParams));
			AniParams.uiFlags         = ANITILE_DOOR | ANITILE_EXISTINGTILE | (fOpenedGraphic ? ANITILE_BACKWARD : ANITILE_FORWARD);
			AniParams.ubLevelID       = ANI_STRUCT_LEVEL;
			AniParams.sStartFrame     = pNode->sCurrentFrame;
			AniParams.sDelay          = INTTILE_DOOR_OPENSPEED;
			AniParams.usTileIndex     = pNode->usIndex;
			AniParams.sGridNo         = sGridNo;
			AniParams.pGivenLevelNode = pNode;
			AniParams.ubKeyFrame1     = pNode->sCurrentFrame + (fOpenedGraphic ? -2 : 2);
	    AniParams.uiKeyFrame1Code = ANI_KEYFRAME_DO_SOUND;
	    AniParams.v.sound         = uiSoundID;
			CreateAnimationTile( &AniParams );
		}

		//if ( pShadowNode != NULL )
		//{
		//	pShadowNode->uiFlags |= LEVELNODE_ANIMATION;
		//	pShadowNode->uiFlags |= LEVELNODE_ANIMATION_PLAYONCE;
		//	pShadowNode->uiFlags |= LEVELNODE_ANIMATION_BACKWARD;
		//	if ( pShadowNode->uiFlags & LEVELNODE_ANIMATION_FORWARD )
		//		pShadowNode->uiFlags ^= LEVELNODE_ANIMATION_FORWARD;
		//	pShadowNode->sDelay		= INTTILE_DOOR_OPENSPEED;
		//}

	}

	if ( fDoAnimation )
	{
		gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
		// FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
		InvalidateWorldRedundency( );

		if ( GridNoOnScreen( sGridNo ) )
		{
			SetRenderFlags(RENDER_FLAG_FULL);
		}
	}

	return( fDoAnimation );
}

void SetDoorString( INT16 sGridNo )
{
	DOOR		*pDoor;
	DOOR_STATUS *		pDoorStatus;
	STRUCTURE *			pStructure;

	BOOLEAN	fTrapped = FALSE;

	// Try and get a door if one exists here
	pDoor = FindDoorInfoAtGridNo( sGridNo );

	if (GetIntTileLocationText() == NULL)
	{
		SetIntTileLocationText(TacticalStr[DOOR_DOOR_MOUSE_DESCRIPTION]);
		if (pDoor != NULL)
		{
			// CHECK PERCEIVED VALUE
			switch( pDoor->bPerceivedTrapped )
			{
				case DOOR_PERCEIVED_TRAPPED:
					SetIntTileLocation2Text(TacticalStr[DOOR_TRAPPED_MOUSE_DESCRIPTION]);
					fTrapped = TRUE;
					break;
			}

			if ( !fTrapped )
			{
				// CHECK PERCEIVED VALUE
				switch( pDoor->bPerceivedLocked )
				{
					case DOOR_PERCEIVED_UNKNOWN:

						break;

					case DOOR_PERCEIVED_LOCKED:
						SetIntTileLocation2Text(TacticalStr[DOOR_LOCKED_MOUSE_DESCRIPTION]);
						break;

					case DOOR_PERCEIVED_UNLOCKED:
						SetIntTileLocation2Text(TacticalStr[DOOR_UNLOCKED_MOUSE_DESCRIPTION]);
						break;

					case DOOR_PERCEIVED_BROKEN:
						SetIntTileLocation2Text(TacticalStr[DOOR_BROKEN_MOUSE_DESCRIPTION]);
						break;
				}
			}

		}
	}

	// ATE: If here, we try to say, opened or closed...
	if (GetIntTileLocation2Text() == NULL)
	{
#ifdef GERMAN
			SetIntTileLocation2Text(TacticalStr[DOOR_DOOR_MOUSE_DESCRIPTION]);

			// Try to get doors status here...
			pDoorStatus = GetDoorStatus( sGridNo );
			if ( pDoorStatus == NULL || ( pDoorStatus != NULL && pDoorStatus->ubFlags & DOOR_PERCEIVED_NOTSET ) )
			{
				// OK, get status based on graphic.....
				pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );
				if (pStructure)
				{
					if ( pStructure->fFlags & STRUCTURE_OPEN )
					{
						// Door is opened....
						SetIntTileLocationText(pMessageStrings[MSG_OPENED]);
					}
					else
					{
						// Door is closed
						SetIntTileLocationText(pMessageStrings[MSG_CLOSED]);
					}
				}
			}
			else
			{
				// Use percived value
				if ( pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN )
				{
					// Door is opened....
					SetIntTileLocationText(pMessageStrings[MSG_OPENED]);
				}
				else
				{
					// Door is closed
					SetIntTileLocationText(pMessageStrings[MSG_CLOSED]);
				}
			}
#else

			// Try to get doors status here...
			pDoorStatus = GetDoorStatus( sGridNo );
			if ( pDoorStatus == NULL || ( pDoorStatus != NULL && pDoorStatus->ubFlags & DOOR_PERCEIVED_NOTSET ) )
			{
				// OK, get status based on graphic.....
				pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );
				if (pStructure)
				{
					if ( pStructure->fFlags & STRUCTURE_OPEN )
					{
						// Door is opened....
						SetIntTileLocation2Text(pMessageStrings[MSG_OPENED]);
					}
					else
					{
						// Door is closed
						SetIntTileLocation2Text(pMessageStrings[MSG_CLOSED]);
					}
				}
			}
			else
			{
				// Use percived value
				if ( pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN )
				{
					// Door is opened....
					SetIntTileLocation2Text(pMessageStrings[MSG_OPENED]);
				}
				else
				{
					// Door is closed
					SetIntTileLocation2Text(pMessageStrings[MSG_CLOSED]);
				}
			}

#endif
	}

}