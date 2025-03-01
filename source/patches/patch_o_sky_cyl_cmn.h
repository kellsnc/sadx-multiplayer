#pragma once

signed int SetCylinderNextAction(taskwk* twp, motionwk2* mwp, playerwk* pwp);

void Mode_SDCylinderStd(taskwk* twp, playerwk* pwp);
void Mode_SDCylinderDown(taskwk* twp, playerwk* pwp);
void Mode_SDCylinderLeft(taskwk* twp, playerwk* pwp);
void Mode_SDCylinderRight(taskwk* twp, playerwk* pwp);

void Mode_SDCylStdChanges(taskwk* twp, playerwk* pwp);
void Mode_SDCylDownChanges(taskwk* twp, playerwk* pwp);