#pragma once

TaskFunc(setDirKaeru, 0x7A6CE0);
TaskFunc(moveKaeru, 0x7A7100);

bool chkAngLimit_m(taskwk* twp, motionwk* mwp, NJS_POINT3* next_pos);
void setDirKaeru3_m(taskwk* twp, motionwk* mwp, BIGETC* etc);
bool chkLureKaeru_m(taskwk* twp, motionwk* mwp, BIGETC* etc);
bool chkDistanceLure_m(taskwk* twp, motionwk* mwp, BIGETC* etc);
bool chkFishPtr_m(task* tp);