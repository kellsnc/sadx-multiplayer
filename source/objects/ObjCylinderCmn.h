#pragma once

signed int SetCylinderNextAction(taskwk* data, motionwk2* data2, playerwk* co2);

void Mode_SDCylinderStd(taskwk* data, playerwk* co2);
void Mode_SDCylinderDown(taskwk* data, playerwk* co2);
void Mode_SDCylinderLeft(taskwk* data, playerwk* co2);
void Mode_SDCylinderRight(taskwk* data, playerwk* co2);

void Mode_SDCylStdChanges(taskwk* data1, playerwk* co2);
void Mode_SDCylDownChanges(taskwk* data1, playerwk* co2);