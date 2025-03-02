#pragma once

signed int SetCylinderNextAction(taskwk* twp, motionwk2* mwp, playerwk* pwp);

void Mode_SDCylinderStd(taskwk* twp, playerwk* pwp);
void Mode_SDCylinderDown(taskwk* twp, playerwk* pwp);
void Mode_SDCylinderLeft(taskwk* twp, playerwk* pwp);
void Mode_SDCylinderRight(taskwk* twp, playerwk* pwp);

void Mode_MD_MULTI_S6A1_WAITChanges(taskwk* twp, playerwk* pwp);
void Mode_MD_MULTI_S6A1_SLIDChanges(taskwk* twp, playerwk* pwp);