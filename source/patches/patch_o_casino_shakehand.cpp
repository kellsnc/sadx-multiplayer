#include "pch.h"

#define get_pnum twp->id

#define CI_ATTR_IGNORE         0x10     /* Do not run physical collision check */
FunctionPointer(int, GetKinkaiNum, (), 0x5C4390);

DataPointer(Float, SAL_casinoshakeanimspd, 0x1E774D8);
DataPointer(Float, SAL_casinoshakegriphandgap, 0x1E774E4);
DataPointer(Float, SAL_casinoshakehandlen, 0x1E774EC);
DataPointer(Float, SAL_casinokinkathrowspd, 0x1E774F8);
DataPointer(Float, SAL_casinokinkascl, 0x1E774FC);
DataPointer(Sint32, SAL_casinokinkanum, 0x1E77500);
DataPointer(Float, SAL_casinokinkathrowspdy, 0x1E774F4);
DataPointer(Float, SAL_casinoshakereleasehandgap, 0x1E774E8);
DataPointer(NJS_MOTION, motion_csn_obj_hand, 0x1E0CA68);

FunctionPointer(int, CalcShakeHandPos, (Float n, NJS_POINT3* vectorB, NJS_POINT3* vd, NJS_POINT3* a4, NJS_POINT3* a5), 0x5CF6F0);
FunctionPointer(Bool, CreateEffKinka1, (NJS_POINT3* pos, NJS_POINT3* spdofs, Float spd, Float reflecty, Float scl), 0x5E0C70);
FunctionPointer(Bool, ObjectCasinoHyoujiki_DispDigit, (Sint32 hyoujikiID, Sint32 num), 0x5D2300);

TaskHook ObjectCasinoShakeHandExec_h(0x5CFA00);


static void ObjectCasinoShakeHandExec(task* tp)
{

	if (multiplayer::IsActive() == false)
	{
		return ObjectCasinoShakeHandExec_h.Original(tp);
	}

	taskwk* twp = tp->twp;
	if (!CheckRangeOutWithR(tp, 62510.0f))
	{
		tp->disp(tp);

		if (!OnEdit(tp))
		{
			if (!++twp->wtimer)
				twp->wtimer = -1;

			colliwk* cwp = twp->cwp;
			cwp->info->attr |= CI_ATTR_IGNORE;
			switch (twp->mode)
			{
			case 0:
			{
				Sint32 pl_id = GetTheNearestPlayerNumber(&twp->pos);
				taskwk* pl_twp = playertwp[pl_id];

				//Not during cooldown, switch is pressed and vault has less than 9999 rings.
				if (twp->wtimer > 100 && GetSwitchOnOff(1) && GetKinkaiNum() < 9999)
				{
					// If Player has 0 rings reset the switch and don't do anything.
					if (GetNumRingM(pl_id) <= 0)
					{
						SetSwitchOnOff(1, 0);
					}
					else
					{
						playerwk* pl_pwp = playerpwp[pl_id];
						pl_pwp->nocontimer += 10;
						SetVelocityP(pl_id, 0.0f, 0.0f, 0.0f);
						SetAutoPilotForBreak((Uint8)pl_id);
						twp->mode = 1;
						twp->wtimer = 0;
						twp->btimer = 0;
						twp->smode = 0;
						get_pnum = pl_id; //pnum
						updateContinueData(&pl_twp->pos, &pl_twp->ang);
					}
				}
				break;
			}
			case 1:
			{
				NJS_POINT3 vec1;
				NJS_POINT3 vec2;
				NJS_POINT3 vec3;
				NJS_POINT3 vd;
				NJS_POINT3 spdofs;
				NJS_POINT3 vec4;
				NJS_POINT3 vec2L;
				NJS_POINT3 vec1L;
				NJS_POINT3 pos1L;
				NJS_POINT3 pos2L;
				NJS_POINT3 vec5;

				taskwk* pl1_twp = playertwp[get_pnum];
				Float counter = twp->counter.f;
				Float frame = (Float)twp->wtimer * SAL_casinoshakeanimspd;
				Float sqrt = -1.0f;
				if (CalcShakeHandPos(frame, &pos1L, &vec1L, &pos2L, &vec2L))
				{
					//Calculate the area the claws can grab Sonic.
					vec1.x = vec1L.x;
					vec1.y = vec1L.y;
					vec1.z = -(vec1L.z * SAL_casinoshakehandlen);

					vec2.y = -(vec2L.y * SAL_casinoshakehandlen);
					vec2.z = -(vec2L.z * SAL_casinoshakehandlen);

					spdofs.x = pos1L.x - vec1L.x * SAL_casinoshakehandlen;
					spdofs.y = pos1L.y - vec1L.y * SAL_casinoshakehandlen;
					spdofs.z = pos1L.z + vec1.z;

					vd.x = pos2L.x - vec2L.x * SAL_casinoshakehandlen;
					vd.y = pos2L.y + vec2.y;
					vd.z = pos2L.z + vec2.z;

					vec3.z = (vd.z + spdofs.z) * 0.5f;
					vec3.x = (vd.x + spdofs.x) * 0.5f;
					vec3.y = (vd.y + spdofs.y) * 0.5f;

					vec4.x = -(vec1L.x * SAL_casinoshakehandlen) - vec2L.x * SAL_casinoshakehandlen;
					vec4.y = vec2.y - vec1L.y * SAL_casinoshakehandlen;
					vec4.z = vec2.z + vec1.z;
					njUnitVector(&vec4);

					vec2 = vec4;
					vec2.x *= playerpwp[get_pnum]->p.height;
					vec2.y *= playerpwp[get_pnum]->p.height;
					vec2.z *= playerpwp[get_pnum]->p.height;

					vec5.x = vec2.x + vec3.x;
					vec5.y = vec2.y + vec3.y;
					vec5.z = vec2.z + vec3.z;

					vec1.x = vec4.x + vec3.x;
					vec1.y = vec4.y + vec3.y;
					vec1.z = vec4.z + vec3.z;

					counter = vec1.y - cwp->info->center.y;
					cwp->info->center = vec1;

					SET_COLLI_RANGE(cwp, njScalor(&cwp->info->center) + 10.0f);
					sqrt = njSqrt((spdofs.x - vd.x) * (spdofs.x - vd.x) + (spdofs.y - vd.y) * (spdofs.y - vd.y) + (spdofs.z - vd.z) * (spdofs.z - vd.z));
				}

				if (!twp->btimer || sqrt <= 0.0f)
				{
					twp->btimer = 0;
					if (twp->value.f > sqrt && sqrt < SAL_casinoshakegriphandgap && (twp->cwp->flag & 1) != 0)
					{
						twp->scl = pl1_twp->pos;
						dsPlay_oneshot(SE_CA_SH_CATCH, 0, 0, 0);
						twp->btimer = 1;
					}

					if (sqrt > 0.0f && frame <= 43.0f && frame >= 30.0f)
						cwp->info->attr &= ~CI_ATTR_IGNORE;

					if (frame > 43.0f && !twp->btimer)
					{
						twp->mode = 3;
						twp->wtimer = 0;
						SetSwitchOnOff(1, 0);
					}

					if (sqrt > 0.0f)
						twp->value.f = sqrt;
				}
				else {
					njPushMatrix(_nj_unit_matrix_);
					njTranslateV(0, &twp->pos);
					ROTATEY(0, twp->ang.y);
					njCalcPoint(0, &vec5, &vec1);
					njCalcVector(0, &vec4, &vd);
					njPopMatrix(1);
					SetRotationP(get_pnum, njArcTan2(njHypot(vd.z, vd.x), vd.y), twp->ang.y, njArcTan2(njHypot(vd.z, vd.x), vd.y));
					SetPositionP(get_pnum, vec1.x, vec1.y, vec1.z);
					if (GetNumRingM(get_pnum) <= 0)
					{
						if (frame >= 57.0f && frame <= 102.0f)
						{
							if (twp->counter.f > 0.0f && counter < 0.0f)
								VibShot(get_pnum, 1);
							if (twp->counter.f < 0.0f && counter > 0.0f)
								VibShot(get_pnum, 1);
						}
					}
					else if (frame >= 57.0f)
					{
						if (frame <= 102.0f)
						{
							Sint32 num = 1;
							Float reflecty = (njRandom() * 20.0f) + playerpwp[get_pnum]->shadow.y_bottom;
							spdofs = { 0.0f, 1.0f, 0.0f };
							CreateEffKinka1(&vec1, &spdofs, SAL_casinokinkathrowspd, reflecty, SAL_casinokinkascl);

							if (twp->counter.f > 0.0f && counter < 0.0f)
							{
								dsPlay_oneshot(SE_DAMAGE, 0, 0, 0);
								dsPlay_oneshot(SE_LEO_BERO, 0, 0, 0);
								vec3 = { 0.0f, twp->counter.f * 0.5f + SAL_casinokinkathrowspdy, 0.0f };
								for (Sint32 i = 0; i < SAL_casinokinkanum; ++i)
								{
									CreateEffKinka1(&vec1, &vec3, SAL_casinokinkathrowspd, reflecty, SAL_casinokinkascl);
								}
								num = SAL_casinokinkanum + 1;
							}

							if (twp->counter.f < 0.0f && counter > 0.0f)
							{
								dsPlay_oneshot(SE_DAMAGE, 0, 0, 0);
								dsPlay_oneshot(SE_LEO_BERO, 0, 0, 0);
								vec3 = { 0.0f, twp->counter.f * 0.5f + SAL_casinokinkathrowspdy, 0.0f };
								for (Sint32 i = 0; i < SAL_casinokinkanum; ++i)
								{
									CreateEffKinka1(&vec1, &vec3, SAL_casinokinkathrowspd, reflecty, SAL_casinokinkascl);
								}
								num += SAL_casinokinkanum;
							}
							if (GetNumRingM(get_pnum) < num)
							{
								num = GetNumRingM(get_pnum);
							}
							AddNumRingM(get_pnum, (Sint16)-num);
							SetCasinoRings(num + GetKinkaiNum());
						}

						if (frame >= 57.0f && frame <= 102.0f)
						{
							if (twp->counter.f > 0.0f && counter < 0.0f)
								VibShot(get_pnum, 1);
							if (twp->counter.f < 0.0f && counter > 0.0f)
								VibShot(get_pnum, 1);
						}
					}

					if (sqrt > 0.0f && sqrt > SAL_casinoshakereleasehandgap)
					{
						//Process Ring count for Player/Vault displays.
						AddNumRingM(get_pnum, -GetNumRingM(get_pnum));
						SetCasinoRings(GetNumRingM(get_pnum) + GetKinkaiNum());
						SetInputP(get_pnum, PL_OP_LETITGO);
						twp->btimer = 0;
						ObjectCasinoHyoujiki_DispDigit(1, GetKinkaiNum());
						ObjectCasinoHyoujiki_DispDigit(2, GetKinkaiNum());
						dsPlay_oneshot(SE_CA_SH_CATCH, 0, 0, 0);
						twp->mode = 2;
						twp->value.f = sqrt;
					}
				}

				twp->counter.f = counter;

				if ((Float)motion_csn_obj_hand.nbFrame - 1.0f <= frame)
				{
					if (twp->btimer)
					{
						SetInputP(get_pnum, PL_OP_LETITGO);
					}
					twp->btimer = 0;
					twp->mode = 0;
					twp->wtimer = 0;
				}
				dsPlay_timer(SE_CA_SH_MOTOR, (Sint32)twp, 1, 0, 5);
				break;
			}
			default:

				ObjectCasinoShakeHandExec_h.Original(tp);
				return;
			}
			EntryColliList(twp);
		}
	}
}

void patch_casino_shakehand_init()
{
	ObjectCasinoShakeHandExec_h.Hook(ObjectCasinoShakeHandExec);
}

RegisterPatch patch_casino_shakehand(patch_casino_shakehand_init);