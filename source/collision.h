#pragma once

extern colaround* around_ring_list_p[];
extern colaround* around_enemy_list_p[];
extern Uint16* arl_num_p[];
extern Uint16* ael_num_p[];

void InitCollisionPatches();
colaround* GetTargetEnemyList(Uint8 pno);
colaround* GetTargetRingList(Uint8 pno);
Uint16 getAelNum(Uint8 pno);
Uint16 getArlNum(Uint8 pno);