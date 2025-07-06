#pragma once


#define PINBALL_FLAG_SLOT_STOPER_C  0x1						/* Slot: Safety block active (Center) */
#define PINBALL_FLAG_SLOT_STOPER_L  0x2						/* Slot: Safety block active (Left) */
#define PINBALL_FLAG_SLOT_STOPER_R  0x4						/* Slot: Safety block active (Right) */
#define PINBALL_FLAG_DTARGET_R		0x8						/* Slot: Arrow pointing to right purple target is active */
#define PINBALL_FLAG_DTARGET_C		0x10					/* Slot: Arrow pointing to middle purple target is active */
#define PINBALL_FLAG_DTARGET_L		0x20					/* Slot: Arrow pointing to left purple target is active  */
#define PINBALL_FLAG_DTARGET_ALL	(PINBALL_FLAG_DTARGET_R|PINBALL_FLAG_DTARGET_C|PINBALL_FLAG_DTARGET_L)
#define PINBALL_FLAG_SLOT_GATE_L	0x40					/* Slot: Left tube gate is open */
#define PINBALL_FLAG_SLOT_GATE_R	0x80					/* Slot: Right tube gate is open */
#define PINBALL_FLAG_STOPER_CAM		0x100					/* Both: No visible effect in-game. See o_stoper for details.*/
#define PINBALL_FLAG_SLOT_TOWER_L	0x200					/* Slot: Entered left tower during Sonic slot bonus */
#define PINBALL_FLAG_SLOT_TOWER_R	0x400					/* Slot: Entered right tower during Sonic slot bonus */
#define PINBALL_FLAG_SLOT_CANNON_L	0x800					/* Slot: Fire left confetti cannon */
#define PINBALL_FLAG_SLOT_CANNON_R	0x1000					/* Slot: Fire right confetti cannon */
#define PINBALL_FLAG_CARD_IDEYA		0x2000					/* Card: Ideya collected (But not given to NiGHTS yet) */
#define PINBALL_FLAG_CARD_DOOR		0x4000					/* Card: Door to boss room open */
#define PINBALL_FLAG_UNUSED			0x8000					/* Not used for anything */
#define PINBALL_FLAG_CARD_THREE		0x10000					/* Card: Three-of-a-Kind bonus active */
#define PINBALL_FLAG_CARD_FHOUSE	0x20000					/* Card: Full House bonus active */
#define PINBALL_FLAG_CARD_FOUR		0x40000					/* Card: Four-of-a-Kind bonus active */
#define PINBALL_FLAG_CARD_STRAIGHT	0x80000					/* Card: Straight bonus active */
#define PINBALL_FLAG_CARD_BONUS		(PINBALL_FLAG_CARD_THREE|PINBALL_FLAG_CARD_FHOUSE|PINBALL_FLAG_CARD_FOUR|PINBALL_FLAG_CARD_STRAIGHT)
#define PINBALL_FLAG_SLOT_SONIC		0x100000				/* Slot: Sonic bonus ("Speed Star") active  */
#define PINBALL_FLAG_SLOT_RING		0x200000				/* Slot: Ring bonus ("Ring 3K") active */
#define PINBALL_FLAG_SLOT_KNUCKLES	0x400000				/* Slot: Knuckles bonus ("Shining 'K'") active */
#define PINBALL_FLAG_SLOT_TAILS		0x800000				/* Slot: Tails bonus ("T-Kickback") active */
#define PINBALL_FLAG_SLOT_AMY		0x1000000				/* Slot: Amy bonus ("Blue Rose") active */
#define PINBALL_FLAG_SLOT_EGGMAN 	0x2000000				/* Slot: Eggman bonus ("Final Eggman") active */
#define PINBALL_FLAG_SLOT_REPLAY 	0x4000000				/* Slot: Activated by hitting all 3 purple targets and deactivated by SetReplayNow. No effect, probably a leftover init flag for it. (old) */
#define PINBALL_FLAG_SLOT_NOTIMERS 	0x8000000				/* Slot: Disables the bonus timers that appear on the stage. Unused in-game, as only the unused purple target camera animation activates it. (old) */
#define PINBALL_FLAG_SLOT_BONUS		(PINBALL_FLAG_SLOT_EGGMAN|PINBALL_FLAG_SLOT_AMY|PINBALL_FLAG_SLOT_TAILS|PINBALL_FLAG_SLOT_KNUCKLES|PINBALL_FLAG_SLOT_RING|PINBALL_FLAG_SLOT_SONIC)
#define PINBALL_FLAG_OBJ_CASINO     0x10000000				/* texlist_obj_casino loaded */
#define PINBALL_FLAG_OBJ_CASINO2    0x20000000				/* texlist_obj_casino2 loaded */
#define PINBALL_FLAG_OBJ_CASINO9    0x40000000				/* texlist_obj_casino9 loaded */
#define PINBALL_FLAG_OBJ_CASINO8    0x80000000				/* texlist_obj_casino8 loaded */

DataPointer(uint8_t, LineKnucklesCounter, 0x3C748E9);
DataPointer(uint8_t, AmyFourCounter, 0x3C748E8);
Float GetBallSpeedM(const uint8_t pnum);

extern uint8_t PinballCameraMode[PLAYER_MAX];