#pragma once

bool CheckDefeat(int pnum);
int GetWinnerMulti();
void SetWinnerMulti(int pnum);

void InitResult();
void MovePlayersToWinnerPos(NJS_VECTOR* endpos);