#pragma once

signed int SetCylinderNextAction(taskwk* data, motionwk2* data2, playerwk* co2);

void init_SDCylinderPatches();
void Mode_SDCylinderStd(taskwk* data, playerwk* co2);
void Mode_SDCylinderDown(taskwk* data, playerwk* co2);
void Mode_SDCylinderLeftRight(taskwk* data, playerwk* co2);