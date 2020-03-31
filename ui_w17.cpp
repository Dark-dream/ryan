#include "stdafx.h"
#if KERNEL
#include "../../HackToolDrv/HackToolDrv/KernelSetting.h"
#endif
#include "GlobalState.h"
#include "UI_Class.h"
#include "DrawData.h"
#include "ui_custom.h"
#include "change_skin.h"
#include "decryptFuncOffset.h"

#if __W5

#define PLAYER_INFO_FONT_SIZE 11	//更低就看不清楚了
#define MENU_FONT_SIZE 10
#define ITEM_FONT_SIZE 12
color_def colors;
color_def* g_color = &colors;

#if 0
#define RADAR_ON RadarMode_Minimap
#else
#define RADAR_ON RadarMode_AllOn
#endif

void init_option()
{
	auto& o = g_state.option;
#if USE_AIM_CROSS_HAIR
	o.AimCrossHair = false;
#endif
	o.ShowLootMode1 = LootEspMode1_AllGun_Backpack;
	o.ShowLootMode2 = LootEspMode2_Ammo_Protect_Heal;
	o.playerEspMode = PlayerEspMode_Basic;
	o.IsShowPlayerBone = true;

	o.IsShowItemBullet = o.IsShowGun = o.IsShowSniper = o.IsShowArmor = true;
	o.IsShowHeal = o.IsShowAttachment = o.IsShowDeadBox = o.IsShowCarepackage = true;
	o.AutoHideItem = true;
	o.IsShowCar = true;

	o.IsShowRadar = RADAR_ON;
	o.WiseTrace = true;
	o.IsShowItemGroupLine = false;

	o.IsShowPlayerBox = o.IsShowPlayerHpBar = o.IsShowPlayerKill = o.IsShowPlayerWeapon =
		o.IsShowPlayerName = o.IsShowPlayerTeam = true;

	o.ItemEspDistance = 100;
	o.AimRange = 250;
}

void init_str_map()
{
	ADD(STR_LOAD_FINISHED, L"清理残留中，请稍等\n");

	const auto heal_color = 0xFF76EE00;
	const auto gun_color1 = 0xFFFFB90F;
	const auto sniper_color1 = 0xFFEE0000;
	const auto bullet_color = 0xFFFFFFFF;
	const auto attachment_color1 = 0xFFC71585;
	const auto attachment_color2 = 0xFFC71585;
	const auto armor_lv1_color = 0xFFEE3A8C;
	const auto armor_lv2_color = 0xFFB0E2FF;
	const auto item_lv3_color = 0xFFFF3030;

	const auto grenade_color = LV1_ITEM_COLOR;
	const auto box_color = 0xFFfcba03;
	const auto quest_color = 0xFFFFFFFF;
	const auto food_color = 0xFFde5b40;
	const auto key_color = 0xFFFF0000;
	const auto barter_color = 0xFFFFFFFF;
	const auto other_color = 0xFFCCCCCC;
	const auto money_color = 0xFFFFFF00;


	g_color->barter_color = barter_color;
	g_color->food_color = food_color;
	g_color->quest_color = quest_color;
	g_color->bullet_color = bullet_color;
	g_color->money_color = money_color;
	g_color->armor_color = armor_lv2_color;
	g_color->attachment_color = attachment_color1;
	g_color->key_color = key_color;

	g_color->common_color = 0xFFffffff;
	g_color->uncommon_color = 0xFF7acf3e;
	g_color->rare_color = 0xFF519de0;
	g_color->epic_color = 0xFFb154e3;
	g_color->legend_color = 0xFFe3c852;
	g_color->player_info_font_size = PLAYER_INFO_FONT_SIZE;
	g_color->menu_font_size = MENU_FONT_SIZE;
	g_color->item_font_size = ITEM_FONT_SIZE;

	DrawData::getInstance().set_text_default_style(ddts_outline);
}



bool is_show_item()
{
	const auto& o = g_state.option;
	return o.IsShowCar | o.IsShowGun | o.IsShowAttachment | o.IsShowArmor | o.IsShowHeal | o.IsShowItemOther | o.IsShowItemQuest | o.IsShowItemBarter | o.IsShowItemMoney |
		o.IsShowDeadBox | o.IsShowCarepackage | o.IsShowPlayerWeapon | o.IsShowThrow | o.AutoHideItem;
}

void func_item_switch()
{
	if (!IsKeyDown(VK_F5)) return;
	const auto o = &g_state.option;
	if (is_show_item())
	{
		o->IsShowCar = o->IsShowItemMoney = o->IsShowItemOther = o->IsShowItemQuest = o->IsShowItemBarter = o->IsShowCar = o->IsShowGun = o->IsShowPlayerWeapon = o->IsShowAttachment = o->IsShowHeal = o->IsShowArmor = o->IsShowThrow = o->IsShowDeadBox = o->AutoHideItem = o->IsShowCarepackage = false;
	}
	else
	{
		o->IsShowCar = o->IsShowItemMoney = o->IsShowItemOther = o->IsShowItemQuest = o->IsShowItemBarter = o->IsShowCar = o->IsShowGun = o->IsShowPlayerWeapon = o->IsShowAttachment = o->IsShowHeal = o->IsShowArmor = o->IsShowThrow = o->IsShowDeadBox = o->AutoHideItem = o->IsShowCarepackage = true;
	}
};

bool is_show_player()
{
	const auto& o = g_state.option;
	return (o.playerEspMode != PlayerEspMode_None) | o.IsShowPlayerBox | o.IsShowXRay | o.IsShowPlayerHpBar | o.IsShowPlayerKill | o.IsShowPlayerTeam | o.IsShowPlayerName;
}

struct menu_item;
using func_color_t = uint32_t(*)();
using func_helper_t = void(*)();
using func_custom_msg_func = clrWstring(*)(const menu_item&);
using func_check_draw_sub_menu_t = bool(*)();

//std::function會產生vtable
struct menu_item
{
	encodedWstring msg;
	//std::function<uint32_t()> func_color;
	func_color_t func_color;
	//std::function<void()> func_check_key;
	//std::function<void()> helper_func;

	func_helper_t func_check_key;
	func_helper_t helper_func;

	//std::function<clrWstring(const menu_item&)> custom_msg_func;
	func_custom_msg_func custom_msg_func;

	std::vector<menu_item>* p_sub_menus = NULL;
	//std::function<bool()> func_check_draw_sub_menu;
	func_check_draw_sub_menu_t func_check_draw_sub_menu;

};

struct menu_define
{
	float x, y;
	std::vector<menu_item>* p_menus;
};

std::vector<menu_define*>* g_menu;

int g_menu_x;
int g_menu_width;
int g_menu_y;
int g_line_height;
int g_sub_menu_y;

int g_panel_x;
int g_panel_y;
int g_panel_width;

#define MENU_COLOR 0xFF598294   //字体
#define MENU_COLOR2 0xFF3460fa  //字体选中
#define MENU_COLOR3 0xFFEEEEEE  
#define MENU_COLOR4 0xFFFF8080
#define MENU_COLOR5 0xFFFF80FF
#define MENU_PANEL_WIDTH 160

#define MENU_PANEL_COLOR 0x20d6f3ff    // MENU底框 
#define MENU_PANEL_BORDER_COLOR 0xFF0f4a63 //MENU外框 
void init_menu()
{
	LA("Enter");
	g_menu = new std::vector<menu_define*>();

	const auto obj = g_data_obj;
	const auto o = &g_state.option;

#define OP_COLOR(a) [](){ const auto o = &g_state.option; return int(a) == 0 ? MENU_COLOR : MENU_COLOR2; }
#define OP_COLOR_CODE(c) [](){ return c; }
#define OP_KEY(a, op) [](){ const auto o = &g_state.option; if(IsKeyDown(a)) { _NOT(op); } }
#define OP_KEY_VAL_INST(a, val_inst) [](){ const auto o = &g_state.option; if(IsKeyDown(a)) { val_inst; } }
#define OP_2K_INST(key1, key2, inst) [](){ const auto o = &g_state.option; if (IsKeyPressing(key1) && IsKeyDown(key2)) { inst; } }
#define OP_2_KEY(a, b, op) [](){ const auto o = &g_state.option; if (IsKeyPressing(a) && IsKeyDown(b)) { _NOT(op); } }
#define OP_2_KEY_2_V(a, b, op1, op2) [](){ const auto o = &g_state.option; if (IsKeyPressing(a) && IsKeyDown(b)) { _NOT(op1); op2 = op1; } }

#define MENU_STR(a)	menu_item{ _strw(a), func_fix_color }
#define MENU_LINE	menu_item{ _strw(L"------------------"), func_fix_color }
#define MENU(msg, key, op) menu_item {_strw(msg),  OP_COLOR(op), OP_KEY(key, op)}
#define MENU_2K(msg, key1, key2, op) menu_item {_strw(msg),  OP_COLOR(op), OP_2_KEY(key1, key2, op)}
#define MENU_2K_INST(msg, key1, key2, op, inst) menu_item {_strw(msg),  OP_COLOR(op), OP_2K_INST(key1, key2, inst)}
#define MENU_2K_2V(msg, key1, key2, op1, op2) menu_item {_strw(msg),  OP_COLOR(op1), OP_2_KEY_2_V(key1, key2, op1, op2)}

	const auto func_fix_color = []() { return MENU_COLOR3; };
	const auto func_before_item_menu = []() { g_sub_menu_y = g_menu_y; };
	const auto func_after_item_menu = []()
	{
		const auto o = &g_state.option;
		if (!o->IsShowMenu) return;
		if (is_show_item())
		{
			DrawData::DrawLine(g_menu_x, g_sub_menu_y, g_menu_x, g_menu_y, MENU_PANEL_BORDER_COLOR);
		}
		g_sub_menu_y = g_menu_y;
	};


	const auto func_draw_line = []()
	{
		const auto o = &g_state.option;
		if (!o->IsShowMenu) return;
		const auto y = int(g_menu_y + g_line_height / 2);
		DrawData::DrawLine(g_panel_x, y, g_panel_x + g_panel_width, y, MENU_PANEL_BORDER_COLOR);
	};

	const auto func_player_switch = []()
	{
		const auto o = &g_state.option;
		if (!IsKeyDown(VK_F4)) return;
		if (is_show_player())
		{
			o->playerEspMode = PlayerEspMode_None;
			o->IsShowPlayerBone = o->IsShowPlayerBox = o->IsShowXRay = o->IsShowPlayerHpBar = o->IsShowPlayerKill = o->IsShowPlayerTeam = o->IsShowPlayerName = false;
		}
		else
		{
			o->playerEspMode = PlayerEspMode_Bone_Name;
			o->IsShowPlayerBone = o->IsShowPlayerBox = o->IsShowXRay = o->IsShowPlayerHpBar = o->IsShowPlayerKill = o->IsShowPlayerTeam = o->IsShowPlayerName = true;
		}
	};

	const auto func_after_player_menu = []()
	{
		const auto o = &g_state.option;
		if (!o->IsShowMenu) return;
		if (is_show_player())
		{
			DrawData::DrawLine(g_menu_x, g_sub_menu_y, g_menu_x, g_menu_y, MENU_PANEL_BORDER_COLOR);
		}
	};

	/*
Solution3

Home 开启菜单
----------------------
~ 战斗模式
F2 雷达地图
F3 自瞄功能
	 自瞄范围 ← →1000
F4 人物透视
	ALT+1 骨骼框框
	ALT+2 人物讯息
   人物加白 PgUp
   重设加白 PgDn
F5 物资显示
	CTRL+1 载具
	CTRL+2 尸体
	物资范围 ↑ ↓ 1000
---------------------
温馨通知:
SHIFT+右键锁头
剩余时间: 6666分钟


menu 底色			d6f3ff
menu border			0f4a63
menu 字体			598294
menu 字体 enable			3460fa
人物近	方框+骨骼		ff0000
人物远	方框+骨骼		3399ff
人物被障碍物遮蔽	方框+骨骼		3399ff
人物	血条		99ff66
人物	武器		d9d9d9
人物	距离+名字		ff0000
人物	组队		0066ff

文字预警			cc0000

物品	普通(垃圾)		ffffff
	一般		339933
	稀有		007bff
	史诗		ea00ff
	传奇		ffc61a
任务	逃离点		339933

载具			d0d0e1
人物被击中			bf1b41
击中显示			击中
	*/

	const auto menu1 = new menu_define{
		0.012f, 0.06f,
		new std::vector<menu_item>
		{
#if 1
			menu_item {_strw(L""), OP_COLOR(false), {}, []()
			{
				const auto o = &g_state.option;
				if (!o->IsShowMenu) return;
				DrawData::DrawString(g_menu_x, g_menu_y, TITLE_FONT_SIZE, MENU_COLOR3, g_state.AppName.Decrypt());
				g_menu_y += 12;
				const auto x_ratio = s_width / 1920;
				const auto padding = int(5 * x_ratio);
				g_panel_x = g_menu_x - padding;
				g_panel_width = g_menu_width + padding * 2;
				g_panel_y = g_menu_y;
			}},
#endif

			MENU(L"Home 开启菜单", VK_HOME, o->IsShowMenu),
			MENU_LINE,
			MENU(L"~ 战斗模式", VK_OEM_3, o->IsBattleMode),
			menu_item {_strw(L"F2 雷达显示"),  OP_COLOR(o->IsShowRadar != RadarMode_AllOff),
				OP_KEY_VAL_INST(VK_F2, o->IsShowRadar = o->IsShowRadar != RadarMode_AllOff ? RadarMode_AllOff : RADAR_ON), {} },
				menu_item {_strw(L"F3 自瞄功能"),  OP_COLOR(o->AimCrossHair), []()
				{
					const auto o = &g_state.option;
					if (IsKeyDown(VK_F3))
					{
						_NOT(o->AimCrossHair);
						o->IsAutoAim = AutoAimMode_None;
						//o->IsInstantHit = false;
						//o->ShotThroughWall = ShotThroughWallMode_Off;
					}
				}
			},
#if 0
			MENU(L"F6 无后压枪", VK_F6, o->IsUsingRecoil)
#endif
#if 0
			MENU(L"F9 弹无扩散", VK_F9, o->IsNoDeviation),
#endif
#if 0
			MENU(L"F7 无限屏息", VK_F7, o->IsNoScopeBreath),


#endif

			menu_item{ _strw(L""),  func_fix_color, []() { aim_range_common(VK_RIGHT, VK_LEFT, 50); }, {},  [](const menu_item& m)
			{
				const auto o = &g_state.option;
				wchar_t buf[64];
				swprintf_s(buf, _strw(L"自瞄范围←→ %d"), o->AimRange);
				return clrWstring(buf);
			} },
			menu_item {_strw(L"F4 人物透视"), OP_COLOR(is_show_player()), func_player_switch, func_after_item_menu, {},
				new std::vector<menu_item>
				{
					MENU_2K_INST(L"ALT+1 骨骼框框", VK_MENU, '1', o->IsShowPlayerBone, _NOT(o->IsShowPlayerBone); o->IsShowPlayerBox= o->IsShowPlayerBone),
				    //MENU_2K(L"Alt+2 框框", VK_MENU, '2', o->IsShowPlayerBox),
				    //MENU_2K(L"Alt+3 血量", VK_MENU, '3', o->IsShowPlayerHpBar),
				    MENU_2K(L"ALT+2 人物讯息", VK_MENU, '2', o->IsShowPlayerName),
			    }, []() { return is_show_player(); }
		    },
			menu_item{ _strw(L"PgUp 人物加白"), func_fix_color, []()
			{
				if (IsKeyDown(VK_PRIOR))
				{
					const auto g = COD_Data::Instance();
					g->add_team_mate(g->current_target);
				}
			} },
			menu_item{ _strw(L"PgDn 重设加白"), func_fix_color, []()
			{
				if (IsKeyDown(VK_NEXT))
				{
					const auto g = COD_Data::Instance();
					g->clear_team_mate();
				}
			} },
			menu_item {_strw(L"F5 物资显示"),  OP_COLOR(is_show_item()), func_item_switch, {}, {}, 
				new std::vector<menu_item>
				{
				    MENU_2K(L"CTRL+1 载具", VK_CONTROL, '1', o->IsShowCar),
				    MENU_2K(L"CTRL+2 尸体", VK_CONTROL, '2', o->IsShowDeadBox),
                    MENU_2K(L"CTRL+3 武器", VK_CONTROL, '3', o->IsShowGun),
					MENU_2K(L"CTRL+4 弹药", VK_CONTROL, '4', o->IsShowItemBullet),
					MENU_2K(L"CTRL+5 其他", VK_CONTROL, '5', o->IsShowItemOther),

			    }, []() { return is_show_item(); }
		    },
	    	menu_item{ _strw(L""),  func_fix_color, []() { item_esp_distance_common(VK_UP, VK_DOWN, 50, 0, 500, 50); }, {},  [](const menu_item& m)
			{
				const auto o = &g_state.option;
				wchar_t buf[64];
				swprintf_s(buf, _strw(L"物资范围 ↑ ↓ %d"), o->ItemEspDistance);
				return clrWstring(buf);
			} },

#if USE_BULLET_TRACE & 0
			menu_item {_strw(L"[F4] 子弹追踪"),  OP_COLOR(o->IsAutoAim == AutoAimMode_BulletTrack), []()
				{
					const auto o = &g_state.option;
					if (IsKeyDown(VK_F4))
					{
						if (o->IsAutoAim == AutoAimMode_BulletTrack)
						{
							o->IsAutoAim = AutoAimMode_None;
						}
						else {
							o->IsAutoAim = AutoAimMode_BulletTrack; o->AimCrossHair = false;
						}
					}
				}
			},
#endif

#if IS_INTERNAL | USE_MINIMAL_MAIN & 0
			menu_item {_strw(L"[F5] 子弹瞬击"),  OP_COLOR(o->IsInstantHit), []()
				{
					const auto o = &g_state.option;
					if (IsKeyDown(VK_F5))
					{
						_NOT(o->IsInstantHit);
					}
				}
			},
#if SHOT_THROUGH_WALL
			menu_item {_strw(L"[F6] 子弹穿墙"),  OP_COLOR(o->ShotThroughWall != ShotThroughWallMode_Off), []()
				{
					const auto o = &g_state.option;
					if (IsKeyDown(VK_F6))
					{
						if (o->ShotThroughWall != ShotThroughWallMode_Off)
						{
							o->ShotThroughWall = ShotThroughWallMode_Off;
						}
						else {
							o->ShotThroughWall = ShotThroughWallMode_All;
						}
					}
				}
			},
#endif
#endif
			
			

#if 0
			menu_item{ _strw(L""),  func_fix_color, []() { item_esp_vaule_common(VK_ADD, VK_SUBTRACT, 10000, 0, 100000, 0); }, {},  [](const menu_item& m)
			{
					const auto o = &g_state.option;
					wchar_t buf[64];
					swprintf_s(buf, _strw(L"物资价值+- %d"), o->ItemValueFilter);
					return clrWstring(buf);
			} },
#endif
#if USE_DRAW_OPACITY
			menu_item{ _strw(L""),  OP_COLOR(false), []() { draw_opacity_common(VK_UP, VK_DOWN, 20); }, {},  [](const menu_item& m)
			{
				const auto o = &g_state.option;
				wchar_t buf[64];
				swprintf_s(buf, _strw(L"[↑/↓] 透视透明 %d"), o->DrawOpacity);
				return clrWstring(buf);
			} },
#endif
				MENU_LINE,
				//menu_item{ _strw(L"CAPS人物穿墙"), OP_COLOR(IsKeyPressing(VK_CAPITAL)), []() { const auto o = &g_state.option; o->JumpWall = IsKeyPressing(VK_CAPITAL); } },
				MENU_STR(L"温馨通知:"),
				MENU_STR(L"SHIFT+右键锁头"),
				//MENU_STR(L"【请注意不要隔墙开枪】"),
				//MENU_STR(L"【游戏巡视员也有透视】"),
				MENU_STR(L""),
				menu_item{ _strw(L""), func_fix_color, {}, {}, [](const menu_item& m)
				{
					if (g_state.isExpired) return _textw(L"剩馀时间结束，请重新购买");
					const auto passedSeconds = (GetTickCount() - g_state.leftSecondStartTick) / 1000;
					const auto leftSecond = g_state.leftSecond - passedSeconds;
					wchar_t buf[64];
					swprintf_s(buf, _strw(L"剩餘时间:%d分钟"), leftSecond / 60);
					return clrWstring(buf);
				} },
				menu_item{ _strw(L""), func_fix_color, []()
				{
					if (!g_state.option.IsShowMenu) return;
					const auto rect_bottom = g_menu_y + g_line_height / 2;
					const auto y = g_panel_y - 10;
					const auto height = rect_bottom - g_panel_y + 8;
					DrawData::FillRect(g_panel_x + 1, y+1, g_panel_width - 2, height-2,MENU_PANEL_COLOR);
					DrawData::DrawRect(g_panel_x, y, g_panel_width, height, MENU_PANEL_BORDER_COLOR);
				}  },
			}
	};
	g_menu->emplace_back(menu1);


	LA("Done");
}

void draw_menu(std::vector<menu_item>* menu)
{
	for (const auto& item : (*menu))
	{
		const auto msg = item.custom_msg_func ? item.custom_msg_func(item) : item.msg.Decrypt();
		const auto color = item.func_color ? item.func_color() : 0;
		if (item.func_check_key) item.func_check_key();
		if (item.helper_func) item.helper_func();
		auto font_size = MENU_FONT_SIZE;
		if (s_width == WIDTH_4K) font_size--;
		if (msg.size() && g_state.option.IsShowMenu) {
			wchar_t mark[2] = { 0 };

			if (color == MENU_COLOR) wcscpy_s(mark, _strw(L"✕"));
			if (color == MENU_COLOR2) wcscpy_s(mark, _strw(L"✓"));

			DrawData::DrawString(g_menu_x, g_menu_y, font_size, color, _textw(L"%s %s"), msg.c_str(), mark);
		}
		g_menu_y += g_line_height;

		if (item.p_sub_menus)
		{
			if (!item.func_check_draw_sub_menu || item.func_check_draw_sub_menu())
			{
				g_menu_x += 20;
				draw_menu(item.p_sub_menus);
				g_menu_x -= 20;
			}
		}
	}
}

void draw_menu()
{
	g_line_height = LINE_HIGHT;
	if (s_width >= WIDTH_4K) {
		g_line_height += 7;
	}
	else if (s_width == WIDTH_2K) {
		g_line_height += 3;
	}

	g_menu_width = int(MENU_PANEL_WIDTH * s_width / 1920.f);

	for (auto menu_def : (*g_menu))
	{
		g_menu_x = int(s_width * menu_def->x);
		g_menu_y = int(s_height * menu_def->y);
		draw_menu(menu_def->p_menus);
	}
}

void check_key()
{
}

void vehicle_key(bool* is_key_enable, bool* is_key_trace)
{
	*is_key_enable = false;
	*is_key_trace = false;
}

void draw_player_info(uint64_t entity, RECT aim_box, bool is_target, bool is_rc_target, bool is_team_mate,
	int distance, float health, uint32_t name_color, clrWstring weapon_name, int team_number, int num_kills, bool is_show_name, clrWstring player_name)
{
	auto& op = g_state.option;
	const auto& name = player_name;
	auto color = name_color;
	auto x_ratio = s_width / 1920.f;
	auto y_ratio = s_height / 1080.f;

	if (op.IsShowPlayerBox && !is_team_mate)
	{
		DrawData::DrawRect(aim_box.left, aim_box.top, aim_box.right - aim_box.left, aim_box.bottom - aim_box.top, color);
	}

	Vector2 screen_pos = { (float)aim_box.left, (float)aim_box.bottom };

	{
		DrawData::DrawString(int(screen_pos.x), int(screen_pos.y), PLAYER_INFO_FONT_SIZE, color,
			_strw(L"[%d]M"), int(distance));
		if (is_show_name)
		{
			DrawData::DrawString(int(screen_pos.x), int(screen_pos.y + 14 * y_ratio), PLAYER_INFO_FONT_SIZE, color,
				_strw(L"%s"), name.c_str());
		}
	}

	GamePlayer& player = *(GamePlayer*)entity;
	if (distance < 50)
	{
		Vector2 spos_head, spos_chest;
		WorldToScreenv2(player.headPos, spos_head);
		WorldToScreenv2(player.chest_pos, spos_chest);

		int rect_size = 6;

		DrawData::DrawRect((int)spos_head.x - rect_size / 2, (int)spos_head.y - rect_size / 2, rect_size, rect_size, color);
#if SHOW_LOG
		DrawData::DrawRect((int)spos_chest.x - rect_size / 2, (int)spos_chest.y - rect_size / 2, rect_size, rect_size, color);
#endif
	}

#if SHOW_LOG

	DrawData::DrawString(int(screen_pos.x), int(screen_pos.y + 28 * y_ratio), PLAYER_INFO_FONT_SIZE, color,
		_str("%d %d %0.1f"), player.stance, player.valid, player.weight);
#endif
#if SHOW_LOG 
	auto target_velocity = player.velocity;
	DrawData::DrawString(int(screen_pos.x), int(screen_pos.y + 42 * y_ratio), PLAYER_INFO_FONT_SIZE, color,
		_str("v:%d %d %d %d"), (int)target_velocity.Size(), (int)target_velocity.x, (int)target_velocity.y, (int)target_velocity.z);
#endif
}


ItemClass item_rule(const clrString& name, int item_id, const item_info_define& item_define, int& size, UINT& color)
{
	const auto& lv = item_define.important_level;
	auto& op = g_state.option;
	if (!op.IsShowItemOther) {
		if (lv == item_is_armor || lv == item_is_heal)
		{
			return ItemClass::Garbage;
		}
	}
	return ItemClass::Important;
}

void draw_warning_info(int spectatedCount, int nearPlayerCount)
{
	/*
	注意300米内有[2]人
最近敌人mingzi[123m]血量[90]击杀[3]
	*/
	constexpr auto warning_info_width = 160;
	constexpr auto warning_info_height = 30;
	const auto warning_info_x = (s_width - warning_info_width) / 2;
	const auto warning_info_y = (int)(s_height * 0.12);

	auto enemyInfoLineHeight = 16;
	int font_size = 14;
	if (s_width == 2560) {
		//warning_info_y -= 28;
		enemyInfoLineHeight += 4;
		font_size++;
	}
	else if (s_width >= WIDTH_4K) {
		//warning_info_y -= 70;
		enemyInfoLineHeight += 10;
		font_size += 2;
	}

	DrawData::DrawString(warning_info_x + 10, warning_info_y, font_size, 0xFFcc0000, _textw(L"注意300米内有[%d]人"), nearPlayerCount);

	uint64_t player;
	clrWstring name;
	int kills;
	int distance;
	int health;
	PUBG_Tool::GetNearPlayerInfo(&player, name, kills, distance, health);
	if (player)
	{
		DrawData::DrawString(warning_info_x - 20, warning_info_y + enemyInfoLineHeight, font_size, 0xFFFFFFFF, _textw(L"最近敌人%s[%dm]血量[%d]击杀[%d]"), name.c_str(), distance / 100, health, kills);
	}

}

clrWstring custom_hit_draw_info (uint32_t& color)
{
	color = 0xFFbf1b41;
	return _textw(L"击中");
}

void ui_init_non_shellcode_func()
{
#if !NO_ESP
	g_ui_define.func_init_str_map = init_str_map;
#endif
	g_ui_define.func_init_option = init_option;
}
void ui_init_shellcode_func()
{
	g_ui_define.func_check_key = check_key;
	g_ui_define.func_vehicle_key = vehicle_key;
#if !NO_ESP
	init_menu();	//新選單系統會用到ShellCode
	g_ui_define.func_draw_menu = draw_menu;

	g_ui_define.func_draw_player_info = draw_player_info;
	g_ui_define.func_draw_warning_info = draw_warning_info;
	g_ui_define.func_custom_item_rule = item_rule;
	g_ui_define.get_hit_draw_info = custom_hit_draw_info;
#endif
}
#endif