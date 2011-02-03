/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Shattrath_City
SD%Complete: 100
SDComment: Quest support: 10004, 10009, 10211, 10231. Flask vendors, Teleport to Caverns of Time
SDCategory: Shattrath City
EndScriptData */

/* ContentData
npc_raliq_the_drunk
npc_salsalabim
npc_shattrathflaskvendors
npc_zephyr
npc_kservant
npc_dirty_larry
npc_ishanah
npc_khadgar
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_raliq_the_drunk
######*/

#define GOSSIP_RALIQ            "You owe Sim'salabim money. Hand them over or die!"

#define FACTION_HOSTILE_RD      45
#define FACTION_FRIENDLY_RD     35

#define SPELL_UPPERCUT          10966

struct TRINITY_DLL_DECL npc_raliq_the_drunkAI : public ScriptedAI
{
    npc_raliq_the_drunkAI(Creature* c) : ScriptedAI(c) {}

    uint32 Uppercut_Timer;

    void Reset()
    {
        Uppercut_Timer = 5000;
        m_creature->setFaction(FACTION_FRIENDLY_RD);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if( Uppercut_Timer < diff )
        {
            DoCast(m_creature->getVictim(),SPELL_UPPERCUT);
            Uppercut_Timer = 15000;
        }else Uppercut_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_raliq_the_drunk(Creature *_Creature)
{
    return new npc_raliq_the_drunkAI (_Creature);
}

bool GossipHello_npc_raliq_the_drunk(Player *player, Creature *_Creature )
{
    if( player->GetQuestStatus(10009) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM(1, GOSSIP_RALIQ, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(9440, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_raliq_the_drunk(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF+1 )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->setFaction(FACTION_HOSTILE_RD);
        ((npc_raliq_the_drunkAI*)_Creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
# npc_salsalabim
######*/

#define FACTION_HOSTILE_SA              90
#define FACTION_FRIENDLY_SA             35
#define QUEST_10004                     10004

#define SPELL_MAGNETIC_PULL             31705

struct TRINITY_DLL_DECL npc_salsalabimAI : public ScriptedAI
{
    npc_salsalabimAI(Creature* c) : ScriptedAI(c) {}

    uint32 MagneticPull_Timer;

    void Reset()
    {
        MagneticPull_Timer = 15000;
        m_creature->setFaction(FACTION_FRIENDLY_SA);
    }

    void EnterCombat(Unit *who) {}

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if( done_by->GetTypeId() == TYPEID_PLAYER )
            if( (m_creature->GetHealth()-damage)*100 / m_creature->GetMaxHealth() < 20 )
        {
            ((Player*)done_by)->GroupEventHappens(QUEST_10004,m_creature);
            damage = 0;
            EnterEvadeMode();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if( MagneticPull_Timer < diff )
        {
            DoCast(m_creature->getVictim(),SPELL_MAGNETIC_PULL);
            MagneticPull_Timer = 15000;
        }else MagneticPull_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_salsalabim(Creature *_Creature)
{
    return new npc_salsalabimAI (_Creature);
}

bool GossipHello_npc_salsalabim(Player *player, Creature *_Creature)
{
    if( player->GetQuestStatus(QUEST_10004) == QUEST_STATUS_INCOMPLETE )
    {
        _Creature->setFaction(FACTION_HOSTILE_SA);
        ((npc_salsalabimAI*)_Creature->AI())->AttackStart(player);
    }
    else
    {
        if( _Creature->isQuestGiver() )
            player->PrepareQuestMenu( _Creature->GetGUID() );
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    }
    return true;
}

/*
##################################################
Shattrath City Flask Vendors provides flasks to people exalted with 3 factions:
Haldor the Compulsive
Arcanist Xorith
Both sell special flasks for use in Outlands 25man raids only,
purchasable for one Mark of Illidari each
Purchase requires exalted reputation with Scryers/Aldor, Cenarion Expedition and The Sha'tar
##################################################
*/

bool GossipHello_npc_shattrathflaskvendors(Player *player, Creature *_Creature)
{
    if(_Creature->GetEntry() == 23484)
    {
        // Aldor vendor
        if( _Creature->isVendor() && (player->GetReputationRank(932) == REP_EXALTED) && (player->GetReputationRank(935) == REP_EXALTED) && (player->GetReputationRank(942) == REP_EXALTED) )
        {
            player->ADD_GOSSIP_ITEM(1, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
            player->SEND_GOSSIP_MENU(11085, _Creature->GetGUID());
        }
        else
        {
            player->SEND_GOSSIP_MENU(11083, _Creature->GetGUID());
        }
    }

    if(_Creature->GetEntry() == 23483)
    {
        // Scryers vendor
        if( _Creature->isVendor() && (player->GetReputationRank(934) == REP_EXALTED) && (player->GetReputationRank(935) == REP_EXALTED) && (player->GetReputationRank(942) == REP_EXALTED) )
        {
            player->ADD_GOSSIP_ITEM(1, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
            player->SEND_GOSSIP_MENU(11085, _Creature->GetGUID());
        }
        else
        {
            player->SEND_GOSSIP_MENU(11084, _Creature->GetGUID());
        }
    }

    return true;
}

bool GossipSelect_npc_shattrathflaskvendors(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if( action == GOSSIP_ACTION_TRADE )
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

/*######
# npc_zephyr
######*/

#define GOSSIP_HZ "Take me to the Caverns of Time."

bool GossipHello_npc_zephyr(Player *player, Creature *_Creature)
{
    if( player->GetReputationRank(989) >= REP_REVERED )
        player->ADD_GOSSIP_ITEM(0, GOSSIP_HZ, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_zephyr(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if( action == GOSSIP_ACTION_INFO_DEF+1 )
        player->CastSpell(player,37778,false);

    return true;
}

/*######
# npc_kservant
######*/

#define SAY1       -1000306
#define WHISP1     -1000307
#define WHISP2     -1000308
#define WHISP3     -1000309
#define WHISP4     -1000310
#define WHISP5     -1000311
#define WHISP6     -1000312
#define WHISP7     -1000313
#define WHISP8     -1000314
#define WHISP9     -1000315
#define WHISP10    -1000316
#define WHISP11    -1000317
#define WHISP12    -1000318
#define WHISP13    -1000319
#define WHISP14    -1000320
#define WHISP15    -1000321
#define WHISP16    -1000322
#define WHISP17    -1000323
#define WHISP18    -1000324
#define WHISP19    -1000325
#define WHISP20    -1000326
#define WHISP21    -1000327

struct TRINITY_DLL_DECL npc_kservantAI : public npc_escortAI
{
public:
    npc_kservantAI(Creature *c) : npc_escortAI(c) {}


    void WaypointReached(uint32 i)
    {
        Player* pTemp = GetPlayerForEscort();
        if( !pTemp )
            return;

        switch(i)
        {
            case 0: DoScriptText(SAY1, m_creature, pTemp); break;
            case 4: DoScriptText(WHISP1, m_creature, pTemp); break;
            case 6: DoScriptText(WHISP2, m_creature, pTemp); break;
            case 7: DoScriptText(WHISP3, m_creature, pTemp); break;
            case 8: DoScriptText(WHISP4, m_creature, pTemp); break;
            case 17: DoScriptText(WHISP5, m_creature, pTemp); break;
            case 18: DoScriptText(WHISP6, m_creature, pTemp); break;
            case 19: DoScriptText(WHISP7, m_creature, pTemp); break;
            case 33: DoScriptText(WHISP8, m_creature, pTemp); break;
            case 34: DoScriptText(WHISP9, m_creature, pTemp); break;
            case 35: DoScriptText(WHISP10, m_creature, pTemp); break;
            case 36: DoScriptText(WHISP11, m_creature, pTemp); break;
            case 43: DoScriptText(WHISP12, m_creature, pTemp); break;
            case 44: DoScriptText(WHISP13, m_creature, pTemp); break;
            case 49: DoScriptText(WHISP14, m_creature, pTemp); break;
            case 50: DoScriptText(WHISP15, m_creature, pTemp); break;
            case 51: DoScriptText(WHISP16, m_creature, pTemp); break;
            case 52: DoScriptText(WHISP17, m_creature, pTemp); break;
            case 53: DoScriptText(WHISP18, m_creature, pTemp); break;
            case 54: DoScriptText(WHISP19, m_creature, pTemp); break;
            case 55: DoScriptText(WHISP20, m_creature, pTemp); break;
            case 56:
                DoScriptText(WHISP21, m_creature, pTemp);
                pTemp->GroupEventHappens(10211,m_creature);
                break;
        }
    }

    void EnterCombat(Unit* who) {}

    void Reset()
    {
        m_creature->SetReactState(REACT_PASSIVE);
        Start(false, false, m_creature->GetCharmerOrOwnerGUID());
    }
};
CreatureAI* GetAI_npc_kservantAI(Creature *_Creature)
{
    npc_kservantAI* kservantAI = new npc_kservantAI(_Creature);

    kservantAI->AddWaypoint(0, -1863.369019, 5419.517090, -10.463668, 4000);
    kservantAI->AddWaypoint(1, -1861.749023, 5416.465332, -10.508068);
    kservantAI->AddWaypoint(2, -1857.036133, 5410.966309, -12.428039);
    kservantAI->AddWaypoint(3, -1831.539185, 5365.472168, -12.428039);
    kservantAI->AddWaypoint(4, -1813.416504, 5333.776855, -12.428039);
    kservantAI->AddWaypoint(5, -1800.354370, 5313.290039, -12.428039);
    kservantAI->AddWaypoint(6, -1775.624878, 5268.786133, -38.809181);
    kservantAI->AddWaypoint(7, -1770.147339, 5259.268066, -38.829231);
    kservantAI->AddWaypoint(8, -1762.814209, 5261.098145, -38.848995);
    kservantAI->AddWaypoint(9, -1740.110474, 5268.858398, -40.208965);
    kservantAI->AddWaypoint(10, -1725.837402, 5270.936035, -40.208965);
    kservantAI->AddWaypoint(11, -1701.580322, 5290.323242, -40.209187);
    kservantAI->AddWaypoint(12, -1682.877808, 5291.406738, -34.429646);
    kservantAI->AddWaypoint(13, -1670.101685, 5291.201172, -32.786007);
    kservantAI->AddWaypoint(14, -1656.666870, 5294.333496, -37.862648);
    kservantAI->AddWaypoint(15, -1652.035767, 5295.413086, -40.245499);
    kservantAI->AddWaypoint(16, -1620.860596, 5300.133301, -40.208992);
    kservantAI->AddWaypoint(17, -1607.630981, 5293.983398, -38.577045, 5000);
    kservantAI->AddWaypoint(18, -1607.630981, 5293.983398, -38.577045, 5000);
    kservantAI->AddWaypoint(19, -1607.630981, 5293.983398, -38.577045, 5000);
    kservantAI->AddWaypoint(20, -1622.140869, 5301.955566, -40.208897);
    kservantAI->AddWaypoint(21, -1625.131836, 5329.112793, -40.208897);
    kservantAI->AddWaypoint(22, -1637.598999, 5342.134277, -40.208790);
    kservantAI->AddWaypoint(23, -1648.521606, 5352.309570, -47.496170);
    kservantAI->AddWaypoint(24, -1654.606934, 5357.419434, -45.870892);
    kservantAI->AddWaypoint(25, -1633.670044, 5422.067871, -42.835541);
    kservantAI->AddWaypoint(25, -1656.567505, 5426.236328, -40.405815);
    kservantAI->AddWaypoint(26, -1664.932373, 5425.686523, -38.846405);
    kservantAI->AddWaypoint(27, -1681.406006, 5425.871094, -38.810928);
    kservantAI->AddWaypoint(28, -1730.875977, 5427.413574, -12.427910);
    kservantAI->AddWaypoint(29, -1743.509521, 5369.599121, -12.427910);
    kservantAI->AddWaypoint(30, -1877.217041, 5303.710449, -12.427989);
    kservantAI->AddWaypoint(31, -1890.371216, 5289.273438, -12.428268);
    kservantAI->AddWaypoint(32, -1905.505737, 5266.534668, 2.630672);
    kservantAI->AddWaypoint(33, -1909.381348, 5273.008301, 1.663714, 10000);
    kservantAI->AddWaypoint(34, -1909.381348, 5273.008301, 1.663714, 12000);
    kservantAI->AddWaypoint(35, -1909.381348, 5273.008301, 1.663714, 8000);
    kservantAI->AddWaypoint(36, -1909.381348, 5273.008301, 1.663714, 15000);
    kservantAI->AddWaypoint(37, -1927.561401, 5275.324707, 1.984987);
    kservantAI->AddWaypoint(38, -1927.385498, 5300.879883, -12.427236);
    kservantAI->AddWaypoint(39, -1921.063965, 5314.318359, -12.427236);
    kservantAI->AddWaypoint(40, -1965.425415, 5379.298828, -12.427236);
    kservantAI->AddWaypoint(41, -1981.233154, 5450.743652, -12.427236);
    kservantAI->AddWaypoint(42, -1958.022461, 5455.904297, 0.487659);
    kservantAI->AddWaypoint(43, -1951.991455, 5463.580566, 0.874490, 10000);
    kservantAI->AddWaypoint(44, -1951.991455, 5463.580566, 0.874490, 12000);
    kservantAI->AddWaypoint(45, -1968.730225, 5481.752930, -12.427846);
    kservantAI->AddWaypoint(46, -1881.839844, 5554.040039, -12.427846);
    kservantAI->AddWaypoint(47, -1841.566650, 5545.965332, -12.427846);
    kservantAI->AddWaypoint(48, -1837.658325, 5523.780273, 0.558756);
    kservantAI->AddWaypoint(49, -1831.321777, 5534.821777, 1.221819, 6000);
    kservantAI->AddWaypoint(50, -1831.321777, 5534.821777, 1.221819, 8000);
    kservantAI->AddWaypoint(51, -1831.321777, 5534.821777, 1.221819, 5000);
    kservantAI->AddWaypoint(52, -1850.060669, 5472.610840, 0.857320, 6000);
    kservantAI->AddWaypoint(53, -1850.060669, 5472.610840, 0.857320, 8000);
    kservantAI->AddWaypoint(54, -1850.060669, 5472.610840, 0.857320, 9000);
    kservantAI->AddWaypoint(55, -1850.060669, 5472.610840, 0.857320, 9000);
    kservantAI->AddWaypoint(56, -1850.060669, 5472.610840, 0.857320, 4000);

    return (CreatureAI*)kservantAI;
}

/*######
# npc_dirty_larry
######*/

#define GOSSIP_BOOK "Ezekiel said that you might have a certain book..."

#define SAY_1       -1000328
#define SAY_2       -1000329
#define SAY_3       -1000330
#define SAY_4       -1000331
#define SAY_5       -1000332
#define SAY_GIVEUP  -1000333

#define QUEST_WBI       10231
#define NPC_CREEPJACK   19726
#define NPC_MALONE      19725

struct TRINITY_DLL_DECL npc_dirty_larryAI : public ScriptedAI
{
    npc_dirty_larryAI(Creature* c) : ScriptedAI(c) 
    {
    m_creature->GetPosition(wLoc);
    }

    bool Event;
    bool Attack;
    bool Done;

    uint64 PlayerGUID;

    uint32 SayTimer;
    uint32 EvadeTimer;
    uint32 Step;
    
    WorldLocation wLoc;

    void Reset()
    {
        Event = false;
        Attack = false;
        Done = false;

        PlayerGUID = 0;
        SayTimer = 0;
        Step = 0;
        EvadeTimer = 3000;

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->setFaction(1194);
        Unit* Creepjack = FindCreature(NPC_CREEPJACK, 20, m_creature);
        if(Creepjack)
        {
            ((Creature*)Creepjack)->AI()->EnterEvadeMode();
            Creepjack->setFaction(1194);
            Creepjack->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);    
        }
        Unit* Malone = FindCreature(NPC_MALONE, 20, m_creature);
        if(Malone)
        {
            ((Creature*)Malone)->AI()->EnterEvadeMode();
            Malone->setFaction(1194);
            Malone->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
    }

    uint32 NextStep(uint32 Step)
    {
        Player *player = Unit::GetPlayer(PlayerGUID);
        
        switch(Step)
        {
        case 0:
        {
            m_creature->SetInFront(player);
            Unit* Creepjack = FindCreature(NPC_CREEPJACK, 20, m_creature);
            if(Creepjack)
                Creepjack->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
            Unit* Malone = FindCreature(NPC_MALONE, 20, m_creature);
            if(Malone)
                Malone->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);    
            m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }return 2000;
        case 1: DoScriptText(SAY_1, m_creature, player); return 3000;
        case 2: DoScriptText(SAY_2, m_creature, player); return 5000;
        case 3: DoScriptText(SAY_3, m_creature, player); return 2000;
        case 4: DoScriptText(SAY_4, m_creature, player); return 2000;
        case 5: DoScriptText(SAY_5, m_creature, player); return 2000;
        case 6: Attack = true; return 2000;
        default: return 0;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(SayTimer < diff)
        {
            if(Event)
                SayTimer = NextStep(++Step);
        }else SayTimer -= diff;

        if(Attack)
        {
            Player *player = Unit::GetPlayer(PlayerGUID);
            m_creature->setFaction(14);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            if(player)
            {
            Unit* Creepjack = FindCreature(NPC_CREEPJACK, 20, m_creature);
            if(Creepjack)
            {
                if(Creepjack->isDead())
                {
                    ((Creature*)Creepjack)->Respawn();
                }
                Creepjack->Attack(player, true);
                Creepjack->setFaction(14);
                Creepjack->GetMotionMaster()->MoveChase(player);
                Creepjack->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
            Unit* Malone = FindCreature(NPC_MALONE, 20, m_creature);
            if(Malone)
            {
                if(Malone->isDead())
                {
                   ((Creature*)Malone)->Respawn();
                }
                Malone->Attack(player, true);
                Malone->setFaction(14);
                Malone->GetMotionMaster()->MoveChase(player);
                Malone->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
                DoStartMovement(player);
                AttackStart(player);
            }
            Attack = false;
        }

        if((m_creature->GetHealth()*100)/m_creature->GetMaxHealth() < 5 && !Done)
        {
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->RemoveAllAuras();
            Unit* Creepjack = FindCreature(NPC_CREEPJACK, 20, m_creature);
            if(Creepjack)
            {
                ((Creature*)Creepjack)->AI()->EnterEvadeMode();
                Creepjack->setFaction(1194);
                Creepjack->GetMotionMaster()->MoveTargetedHome();
                Creepjack->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
            Unit* Malone = FindCreature(NPC_MALONE, 20, m_creature);
            if(Malone)
            {
                ((Creature*)Malone)->AI()->EnterEvadeMode();
                Malone->setFaction(1194);
                Malone->GetMotionMaster()->MoveTargetedHome();
                Malone->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
            m_creature->setFaction(1194);
            Done = true;
            DoScriptText(SAY_GIVEUP, m_creature, NULL);
            m_creature->DeleteThreatList();
            m_creature->CombatStop();
            m_creature->GetMotionMaster()->MoveTargetedHome();
            Player* player = Unit::GetPlayer(PlayerGUID);
            if(player)
                player->GroupEventHappens(QUEST_WBI, m_creature);
            Reset();
        }
        if(EvadeTimer < diff)
        {
                if(m_creature->GetDistance2d(wLoc.coord_x, wLoc.coord_y) >= 50)
                EnterEvadeMode();
                EvadeTimer = 3000;
                return;
        }
        else EvadeTimer -= diff; 
        DoMeleeAttackIfReady();
    }
};

bool GossipHello_npc_dirty_larry(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if(player->GetQuestStatus(QUEST_WBI) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, GOSSIP_BOOK, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_dirty_larry(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        ((npc_dirty_larryAI*)creature->AI())->Event = true;
        ((npc_dirty_larryAI*)creature->AI())->PlayerGUID = player->GetGUID();
        player->CLOSE_GOSSIP_MENU();
    }

    return true;
}

CreatureAI* GetAI_npc_dirty_larryAI(Creature *_Creature)
{
    return new npc_dirty_larryAI (_Creature);
}

/*######
# npc_ishanah
######*/

#define ISANAH_GOSSIP_1 "Who are the Sha'tar?"
#define ISANAH_GOSSIP_2 "Isn't Shattrath a draenei city? Why do you allow others here?"

bool GossipHello_npc_ishanah(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    player->ADD_GOSSIP_ITEM(0, ISANAH_GOSSIP_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->ADD_GOSSIP_ITEM(0, ISANAH_GOSSIP_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_ishanah(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if(action == GOSSIP_ACTION_INFO_DEF+1)
        player->SEND_GOSSIP_MENU(9458, _Creature->GetGUID());
    else if(action == GOSSIP_ACTION_INFO_DEF+2)
        player->SEND_GOSSIP_MENU(9459, _Creature->GetGUID());

    return true;
}

/*######
# npc_khadgar
######*/

#define KHADGAR_GOSSIP_1    "I've heard your name spoken only in whispers, mage. Who are you?"
#define KHADGAR_GOSSIP_2    "Go on, please."
#define KHADGAR_GOSSIP_3    "I see." //6th too this
#define KHADGAR_GOSSIP_4    "What did you do then?"
#define KHADGAR_GOSSIP_5    "What happened next?"
#define KHADGAR_GOSSIP_7    "There was something else I wanted to ask you."

bool GossipHello_npc_khadgar(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if(!player->hasQuest(10211))
        player->ADD_GOSSIP_ITEM(0, KHADGAR_GOSSIP_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(9243, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_khadgar(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch(action)
    {
    case GOSSIP_ACTION_INFO_DEF+1:
        player->ADD_GOSSIP_ITEM(0, KHADGAR_GOSSIP_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        player->SEND_GOSSIP_MENU(9876, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+2:
        player->ADD_GOSSIP_ITEM(0, KHADGAR_GOSSIP_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
        player->SEND_GOSSIP_MENU(9877, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+3:
        player->ADD_GOSSIP_ITEM(0, KHADGAR_GOSSIP_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
        player->SEND_GOSSIP_MENU(9878, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+4:
        player->ADD_GOSSIP_ITEM(0, KHADGAR_GOSSIP_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
        player->SEND_GOSSIP_MENU(9879, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+5:
        player->ADD_GOSSIP_ITEM(0, KHADGAR_GOSSIP_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
        player->SEND_GOSSIP_MENU(9880, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+6:
        player->ADD_GOSSIP_ITEM(0, KHADGAR_GOSSIP_7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
        player->SEND_GOSSIP_MENU(9881, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+7:
        player->ADD_GOSSIP_ITEM(0, KHADGAR_GOSSIP_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(9243, creature->GetGUID());
        break;
    }
    return true;
}

/*######
 # Event after rewarding quest Kael'thas and the Verdant Sphere
 ######*/

#define QUEST_VERDANT_SPHERE                11007

#define GO_FIRE                             185170
#define CREATURE_ADAL                       18481
#define CREATURE_KAEL                       23054

#define SPELL_ADALS_SONG_OF_BATTLE          39953
#define SPELL_KAELTHAS_DEFEATED             39966
#define SPELL_OTHERWORLDLY_PORTAL           39952
#define SPELL_MARK_OF_KAELTHAS              37364

#define ADAL_WHISPER1                       "Silence descends upon Shattrath"
#define ADAL_WHISPER2                       "A'dal's thoughts invade your mind"
#define ADAL_WHISPER3_1                     "Kael'thas Sunstrider has been defeated by "
#define ADAL_WHISPER3_2                     " and "
#define ADAL_WHISPER3_3                     " allies"
#define ADAL_WHISPER4                       "The time to strike at the remaining blood elves of Tempest Keep is now. Take arms and let A'dal's song of battle empower you!"
#define KAEL_YELL1                          "Your monkeys failed to finish the job, naaru! Beaten but alive... The same mistake was not made when we took command of your vessel."
#define KAEL_YELL2                          "All for what? Trinkets? You are too late. The preparations have already begun. Soon the master will make his return."
#define KAEL_YELL3                          "And there is nothing you or that fool, Illidan, can do to stop me! You have both served me in your own right - unwillingly."
#define KAEL_YELL4                          "Lay down your arms and succumb to the might of Kil'jaeden!"



struct TRINITY_DLL_DECL npc_kaelthas_imageAI : public ScriptedAI
{
    npc_kaelthas_imageAI(Creature* c) : ScriptedAI(c) {}

    uint8 Step;
    uint32 NextStep_Timer;
    std::list<uint64> PlayersInCity;
    bool Init;
    std::string Defeater_Name;
    std::string Defeater_Gender;
    uint64 FireGO;

    void Reset()
    {
        Step = 0;
        NextStep_Timer = 2000;
        PlayersInCity.clear();
        Init = false;
        FireGO = 0;
    }

    void EmoteTo(Creature* sender, const char *text, Player *target)
    {
        WorldPacket data(SMSG_MESSAGECHAT, 200);
        sender->BuildMonsterChat(&data,CHAT_MSG_MONSTER_EMOTE,text,0,sender->GetName(), target->GetGUID(), true);
        target->GetSession()->SendPacket(&data);
    }

    void YellTo(Creature* sender, const char* text, Player *target)
    {
        WorldPacket data(SMSG_MESSAGECHAT, 200);
        sender->BuildMonsterChat(&data,CHAT_MSG_MONSTER_YELL,text,0,sender->GetName(), target->GetGUID());
        target->GetSession()->SendPacket(&data);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!Init)
        {     
            std::list<Unit*> PlayerList;
            uint32 shattrathRadius = 1000;
            Trinity::AnyUnitInObjectRangeCheck  check(me, shattrathRadius);
            Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck > searcher(PlayerList, check);
            Cell::VisitWorldObjects(me, searcher, shattrathRadius);

            for(std::list<Unit*>::iterator i = PlayerList.begin(); i != PlayerList.end(); i++)
            {
                if((*i)->GetTypeId() != TYPEID_PLAYER || (*i)->GetZoneId() != 3703)
                    continue;
                PlayersInCity.push_back((*i)->GetGUID());
            }

            me->AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT | MOVEMENTFLAG_LEVITATING);
            me->SetVisibility(VISIBILITY_OFF);
            me->StopMoving();
            //DoCast(me, SPELL_OTHERWORLDLY_PORTAL, true);
            DoCast(me, SPELL_KAELTHAS_DEFEATED, true);
            //DoCast(me, SPELL_MARK_OF_KAELTHAS, true);
            Init = true;
        }

        if(NextStep_Timer < diff)
        {
            NextStep_Timer = 13000;
            Creature* adal = NULL;
            
            if(Step < 4) 
            {
                if(Unit *u = FindCreature(CREATURE_ADAL, 100, me))
                    if(u->GetTypeId() == TYPEID_UNIT)
                        adal = (Creature*)u;

                if(!adal)
                    return;
            }

            std::string s;            
            switch(Step)
            {
                
                case 0:
                    for(std::list<uint64>::iterator i = PlayersInCity.begin(); i != PlayersInCity.end(); i++)
                        if(Unit *u = me->GetUnit(*me, (*i)))
                            EmoteTo(adal, ADAL_WHISPER1, (Player*)u);
                    NextStep_Timer = 7000;
                    break;
                case 1:
                    for(std::list<uint64>::iterator i = PlayersInCity.begin(); i != PlayersInCity.end(); i++)
                        if(Unit *u = me->GetUnit(*me, (*i)))
                            EmoteTo(adal, ADAL_WHISPER2, (Player*)u);
                    NextStep_Timer = 7000;
                    break;
                case 2:
                    s = std::string() + ADAL_WHISPER3_1 + Defeater_Name + ADAL_WHISPER3_2 + Defeater_Gender + ADAL_WHISPER3_3;
                    for(std::list<uint64>::iterator i = PlayersInCity.begin(); i != PlayersInCity.end(); i++)
                        if(Unit *u = me->GetUnit(*me, (*i)))
                            YellTo(adal, s.c_str(), (Player*)u);
                    break;
                case 3:
                    for(std::list<uint64>::iterator i = PlayersInCity.begin(); i != PlayersInCity.end(); i++)
                    {
                        if(Unit *u = me->GetUnit(*me, (*i)))
                        {
                            u->CastSpell(u, 39953, true);
                            YellTo(adal, ADAL_WHISPER4, (Player*)u);
                        }
                    }
                    me->SetVisibility(VISIBILITY_ON);
                    break;
                case 4:
                    me->Yell(KAEL_YELL1, 0, 0);
                    break;
                case 5:
                    me->Yell(KAEL_YELL2, 0, 0);
                    break;
                case 6:
                    me->Yell(KAEL_YELL3, 0, 0);
                    break;
                case 7:
                    me->Yell(KAEL_YELL4, 0, 0);
                    NextStep_Timer = 6000;
                    break;
                case 8:
                    if(FireGO)
                        if(GameObject* fire = me->GetMap()->GetGameObject(FireGO))
                            fire->Delete();
                    break;

            }
            Step++;
        }
        else
            NextStep_Timer -= diff;

    }
};

CreatureAI* GetAI_npc_kaelthas_imageAI(Creature *_Creature)
{
    return new npc_kaelthas_imageAI (_Creature);
}

bool ChooseReward_npc_Adal(Player *player, Creature *_Creature, const Quest *_Quest, uint32 slot)
{
    if(_Quest->GetQuestId() == QUEST_VERDANT_SPHERE)
    {
        GameObject *fire = _Creature->SummonGameObject(185170, -1831.9, 5429.7, -1.5, 0, 0, 0, 0, 0, 0);
        Creature* kael = _Creature->SummonCreature( CREATURE_KAEL, -1839.6, 5429.8, -1.5, 3.1214, TEMPSUMMON_TIMED_DESPAWN, 100000);
        
        ((npc_kaelthas_imageAI*)kael->AI())->Defeater_Name = player->GetName();
        ((npc_kaelthas_imageAI*)kael->AI())->Defeater_Gender = player->getGender() == GENDER_MALE ? "his" : "her";
        if(fire)
            ((npc_kaelthas_imageAI*)kael->AI())->FireGO = fire->GetGUID();
    }
    return false;
}


void AddSC_shattrath_city()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_raliq_the_drunk";
    newscript->pGossipHello =  &GossipHello_npc_raliq_the_drunk;
    newscript->pGossipSelect = &GossipSelect_npc_raliq_the_drunk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_salsalabim";
    newscript->GetAI = &GetAI_npc_salsalabim;
    newscript->pGossipHello =  &GossipHello_npc_salsalabim;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_shattrathflaskvendors";
    newscript->pGossipHello =  &GossipHello_npc_shattrathflaskvendors;
    newscript->pGossipSelect = &GossipSelect_npc_shattrathflaskvendors;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_zephyr";
    newscript->pGossipHello =  &GossipHello_npc_zephyr;
    newscript->pGossipSelect = &GossipSelect_npc_zephyr;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_kservant";
    newscript->GetAI = &GetAI_npc_kservantAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_dirty_larry";
    newscript->GetAI = &GetAI_npc_dirty_larryAI;
    newscript->pGossipHello =   &GossipHello_npc_dirty_larry;
    newscript->pGossipSelect = &GossipSelect_npc_dirty_larry;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ishanah";
    newscript->pGossipHello =  &GossipHello_npc_ishanah;
    newscript->pGossipSelect = &GossipSelect_npc_ishanah;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_khadgar";
    newscript->pGossipHello =  &GossipHello_npc_khadgar;
    newscript->pGossipSelect = &GossipSelect_npc_khadgar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_kaelthas_image";
    newscript->GetAI = &GetAI_npc_kaelthas_imageAI;
    newscript->RegisterSelf();
 
    newscript = new Script;
    newscript->Name="npc_adal";
    newscript->pChooseReward = &ChooseReward_npc_Adal;
    newscript->RegisterSelf();
}

