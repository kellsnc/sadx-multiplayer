#pragma once


typedef struct bossextwk2
{
	bosswk bwk;
	Sint32 top_y;
	Sint32 top_target_y;
	Sint32 missile_pod_x;
	Sint32 missile_pod_target_x;
	NJS_POINT3 missile_speed;
	Float missile_power;
	Sint32 fire_top_time;
	Sint32 fire_pod_time;
	Sint32 fire_pod_x;
	Sint32 fire_pod_target_x;
	Uint8 fire_top_rot_cnt;
	Uint8 fire_pod_rot_cnt;
	NJS_POINT3 speed;
	NJS_POINT3 target;
	Float move_rate;
	Uint8 move_mode;
	Uint8 jump_mode;
	Uint8 escape_mode;
	Float escape_range;
	Uint8 weak_mode[4];
	Float leg_mat[4][16];
	Uint8 leg_mode[4];
	Float HitPoint;
}bossextwk2;