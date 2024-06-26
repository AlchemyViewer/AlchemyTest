/**
 * @file llpanelpeoplemenus.h
 * @brief Menus used by the side tray "People" panel
 *
 * $LicenseInfo:firstyear=2009&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

// libs
#include "llmenugl.h"
#include "lluictrlfactory.h"

#include "llpanelpeoplemenus.h"

// newview
#include "alavataractions.h"
#include "llagent.h"
#include "llagentdata.h"            // for gAgentID
#include "llavataractions.h"
#include "llcallingcard.h"          // for LLAvatarTracker
#include "llfloaterreporter.h"
#include "lllogchat.h"
#include "llparcel.h"
#include "llviewermenu.h"           // for gMenuHolder
#include "llconversationmodel.h"
#include "llviewerobjectlist.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "llvoavatarself.h"
#include "roles_constants.h"
// [RLVa:KB] - Checked: RLVa-2.0.1
#include "rlvactions.h"
// [/RLVa:KB]

namespace LLPanelPeopleMenus
{

PeopleContextMenu gPeopleContextMenu;
NearbyPeopleContextMenu gNearbyPeopleContextMenu;

//== PeopleContextMenu ===============================================================

LLContextMenu* PeopleContextMenu::createMenu()
{
    // set up the callbacks for all of the avatar menu items
    LLUICtrl::CommitCallbackRegistry::ScopedRegistrar registrar;
    LLUICtrl::EnableCallbackRegistry::ScopedRegistrar enable_registrar;
    LLContextMenu* menu;

    if ( mUUIDs.size() == 1 )
    {
        // Set up for one person selected menu

        const LLUUID& id = mUUIDs.front();
        registrar.add("Avatar.Profile",         boost::bind(&LLAvatarActions::showProfile,              id));
        registrar.add("Avatar.AddFriend",       boost::bind(&LLAvatarActions::requestFriendshipDialog,  id));
        registrar.add("Avatar.RemoveFriend",    boost::bind(&LLAvatarActions::removeFriendDialog,       id));
        registrar.add("Avatar.IM",              boost::bind(&LLAvatarActions::startIM,                  id));
        registrar.add("Avatar.Call",            boost::bind(&LLAvatarActions::startCall,                id));
        registrar.add("Avatar.OfferTeleport",   boost::bind(&PeopleContextMenu::offerTeleport,          this));
        registrar.add("Avatar.ZoomIn",          boost::bind(&ALAvatarActions::zoomIn,                   id));
        registrar.add("Avatar.ShowOnMap",       boost::bind(&LLAvatarActions::showOnMap,                id));
        registrar.add("Avatar.Share",           boost::bind(&LLAvatarActions::share,                    id));
        registrar.add("Avatar.Pay",             boost::bind(&LLAvatarActions::pay,                      id));
        registrar.add("Avatar.BlockUnblock",    boost::bind(&LLAvatarActions::toggleBlock,              id));
        registrar.add("Avatar.InviteToGroup",   boost::bind(&LLAvatarActions::inviteToGroup,            id));
        registrar.add("Avatar.TeleportRequest", boost::bind(&PeopleContextMenu::requestTeleport,        this));
        registrar.add("Avatar.Calllog",         boost::bind(&LLAvatarActions::viewChatHistory,          id));
        registrar.add("Avatar.Freeze",          [&](LLUICtrl*, const LLSD& param) { ALAvatarActions::parcelFreeze(id);});
        registrar.add("Avatar.Eject",           [&](LLUICtrl*, const LLSD&) { ALAvatarActions::parcelEject(id); });
        registrar.add("Avatar.CopyData",        [&](LLUICtrl*, const LLSD& param) { ALAvatarActions::copyData(id, param);});
        registrar.add("Avatar.ManageEstate", [&](LLUICtrl*, const LLSD& param) {
            switch (param.asInteger())
            {
            case 0: ALAvatarActions::estateTeleportHome(id); break;
            case 1: ALAvatarActions::estateKick(id); break;
            case 2: ALAvatarActions::estateBan(id); break;
            } });
        registrar.add("Avatar.TeleportTo", [&](LLUICtrl*, const LLSD&) { ALAvatarActions::teleportTo(id); });
        registrar.add("Avatar.ReportAbuse", [&](LLUICtrl*, const LLSD&) { ALAvatarActions::reportAbuse(id); });

        enable_registrar.add("Avatar.EnableItem", boost::bind(&PeopleContextMenu::enableContextMenuItem, this, _2));
        enable_registrar.add("Avatar.CheckItem",  boost::bind(&PeopleContextMenu::checkContextMenuItem, this, _2));

        // create the context menu from the XUI
        menu = createFromFile("menu_people_nearby.xml");
        buildContextMenu(*menu, 0x0);
    }
    else
    {
        // Set up for multi-selected People

        // registrar.add("Avatar.AddFriend",    boost::bind(&LLAvatarActions::requestFriendshipDialog,  mUUIDs)); // *TODO: unimplemented
        registrar.add("Avatar.IM",              boost::bind(&PeopleContextMenu::startConference,        this));
        registrar.add("Avatar.Call",            boost::bind(&LLAvatarActions::startAdhocCall,           mUUIDs, LLUUID::null));
        registrar.add("Avatar.OfferTeleport",   boost::bind(&PeopleContextMenu::offerTeleport,          this));
        registrar.add("Avatar.RemoveFriend",    boost::bind(&LLAvatarActions::removeFriendsDialog,      mUUIDs));
        // registrar.add("Avatar.Share",        boost::bind(&LLAvatarActions::startIM,                  mUUIDs)); // *TODO: unimplemented
        // registrar.add("Avatar.Pay",          boost::bind(&LLAvatarActions::pay,                      mUUIDs)); // *TODO: unimplemented
        registrar.add("Avatar.CopyData",        [&](LLUICtrl*, const LLSD& param) { ALAvatarActions::copyData(mUUIDs, param); });
        registrar.add("Avatar.Freeze",          [&](LLUICtrl*, const LLSD&) { ALAvatarActions::parcelFreeze(mUUIDs); });
        registrar.add("Avatar.Eject",           [&](LLUICtrl*, const LLSD&) { ALAvatarActions::parcelEject(mUUIDs); });
        registrar.add("Avatar.ManageEstate", [&](LLUICtrl*, const LLSD& param) {
            switch (param.asInteger())
            {
            case 0: ALAvatarActions::estateTeleportHome(mUUIDs); break;
            case 1: ALAvatarActions::estateKick(mUUIDs); break;
            case 2: ALAvatarActions::estateBan(mUUIDs); break;
            } });

        enable_registrar.add("Avatar.EnableItem",   boost::bind(&PeopleContextMenu::enableContextMenuItem, this, _2));

        // create the context menu from the XUI
        menu = createFromFile("menu_people_nearby_multiselect.xml");
        buildContextMenu(*menu, ITEM_IN_MULTI_SELECTION);
    }

    return menu;
}

void PeopleContextMenu::buildContextMenu(class LLMenuGL& menu, U32 flags)
{
    menuentry_vec_t items;
    menuentry_vec_t disabled_items;

    if (flags & ITEM_IN_MULTI_SELECTION)
    {
        items.push_back(std::string("add_friends"));
        items.push_back(std::string("remove_friends"));
        items.push_back(std::string("im"));
        items.push_back(std::string("call"));
        items.push_back(std::string("share"));
        items.push_back(std::string("pay"));
        items.push_back(std::string("offer_teleport"));
        items.push_back(std::string("separator_utils"));
        items.push_back(std::string("utils_menu"));
        items.push_back(std::string("copy_full_name"));
        items.push_back(std::string("copy_display_name"));
        items.push_back(std::string("copy_username"));
        items.push_back(std::string("copy_slurl"));
        items.push_back(std::string("copy_uuid"));
    }
    else
    {
        items.push_back(std::string("view_profile"));
        items.push_back(std::string("im"));
        items.push_back(std::string("offer_teleport"));
        items.push_back(std::string("request_teleport"));
        items.push_back(std::string("voice_call"));
        items.push_back(std::string("chat_history"));
        items.push_back(std::string("separator_chat_history"));
        items.push_back(std::string("add_friend"));
        items.push_back(std::string("remove_friend"));
        items.push_back(std::string("invite_to_group"));
        items.push_back(std::string("separator_invite_to_group"));
        items.push_back(std::string("map"));
        items.push_back(std::string("share"));
        items.push_back(std::string("pay"));
        items.push_back(std::string("separator_utils"));
        items.push_back(std::string("manage_menu"));
        items.push_back(std::string("block_unblock"));
        items.push_back(std::string("report_abuse"));
        items.push_back(std::string("utils_menu"));
        items.push_back(std::string("copy_full_name"));
        items.push_back(std::string("copy_display_name"));
        items.push_back(std::string("copy_username"));
        items.push_back(std::string("copy_slurl"));
        items.push_back(std::string("copy_uuid"));
    }

    hide_context_entries(menu, items, disabled_items);
}

bool PeopleContextMenu::enableContextMenuItem(const LLSD& userdata)
{
    if(gAgent.getID() == mUUIDs.front())
    {
        return false;
    }
    std::string item = userdata.asString();

    // Note: can_block and can_delete is used only for one person selected menu
    // so we don't need to go over all uuids.

    if (item == std::string("can_block"))
    {
        const LLUUID& id = mUUIDs.front();
        return LLAvatarActions::canBlock(id);
    }
    else if (item == std::string("can_add"))
    {
        // We can add friends if:
        // - there are selected people
        // - and there are no friends among selection yet.

        //EXT-7389 - disable for more than 1
        if(mUUIDs.size() > 1)
        {
            return false;
        }

        bool result = (mUUIDs.size() > 0);

        uuid_vec_t::const_iterator
            id = mUUIDs.begin(),
            uuids_end = mUUIDs.end();

        for (;id != uuids_end; ++id)
        {
//          if ( LLAvatarActions::isFriend(*id) )
// [RLVa:KB] - Checked: 2014-03-31 (RLVa-2.0.1)
            if ( (LLAvatarActions::isFriend(*id)) || (!RlvActions::canShowName(RlvActions::SNC_DEFAULT, *id)) )
// [/RLVa:KB]
            {
                result = false;
                break;
            }
        }

        return result;
    }
    else if (item == std::string("can_delete"))
    {
        // We can remove friends if:
        // - there are selected people
        // - and there are only friends among selection.

        bool result = (mUUIDs.size() > 0);

        uuid_vec_t::const_iterator
            id = mUUIDs.begin(),
            uuids_end = mUUIDs.end();

        for (;id != uuids_end; ++id)
        {
//          if ( !LLAvatarActions::isFriend(*id) )
// [RLVa:KB] - Checked: 2014-03-31 (RLVa-2.0.1)
            if ( (!LLAvatarActions::isFriend(*id)) || (!RlvActions::canShowName(RlvActions::SNC_DEFAULT, *id)) )
// [/RLVa:KB]
            {
                result = false;
                break;
            }
        }

        return result;
    }
    else if (item == std::string("can_call"))
    {
        return LLAvatarActions::canCall();
    }
    else if (item == std::string("can_zoom_in"))
    {
        const LLUUID& id = mUUIDs.front();

        return ALAvatarActions::canZoomIn(id);
    }
    else if (item == std::string("can_show_on_map"))
    {
        const LLUUID& id = mUUIDs.front();

        return (LLAvatarTracker::instance().isBuddyOnline(id) && LLAvatarActions::isAgentMappable(id))
                    || gAgent.isGodlike();
    }
    else if(item == std::string("can_offer_teleport"))
    {
        return LLAvatarActions::canOfferTeleport(mUUIDs);
    }
    else if (item == std::string("can_callog"))
    {
        return LLLogChat::isTranscriptExist(mUUIDs.front());
    }
// [RLVa:KB] - @pay
    else if (item == std::string("can_pay"))
    {
        return RlvActions::canPayAvatar(mUUIDs.front());
    }
// [/RLVa:KB]
    else if (item == std::string("can_im") || item == std::string("can_invite") ||
             item == std::string("can_share") || item == std::string("can_pay"))
    {
        return true;
    }
    else if (item == std::string("can_teleport_to"))
    {
        return ALAvatarActions::canTeleportTo(mUUIDs.front());
    }
    else if (item == std::string("can_freeze_eject"))
    {
        return ALAvatarActions::canFreezeEject(mUUIDs);
    }
    else if (item == std::string("can_manage_estate"))
    {
        return ALAvatarActions::canManageAvatarsEstate(mUUIDs);
    }
    return false;
}

bool PeopleContextMenu::checkContextMenuItem(const LLSD& userdata)
{
    std::string item = userdata.asString();
    const LLUUID& id = mUUIDs.front();

    if (item == std::string("is_blocked"))
    {
        return LLAvatarActions::isBlocked(id);
    }

    return false;
}

void PeopleContextMenu::requestTeleport()
{
    // boost::bind cannot recognize overloaded method LLAvatarActions::teleportRequest(),
    // so we have to use a wrapper.
// [RLVa:KB] - Checked: RLVa-2.0.1
    bool fRlvCanShowName = (!m_fRlvCheck) || (RlvActions::canShowName(RlvActions::SNC_DEFAULT, mUUIDs.front()));
    RlvActions::setShowName(RlvActions::SNC_TELEPORTREQUEST, fRlvCanShowName);
    LLAvatarActions::teleportRequest(mUUIDs.front());
    RlvActions::setShowName(RlvActions::SNC_TELEPORTREQUEST, true);
// [/RLVa:KB]
//  LLAvatarActions::teleportRequest(mUUIDs.front());
}

void PeopleContextMenu::offerTeleport()
{
    // boost::bind cannot recognize overloaded method LLAvatarActions::offerTeleport(),
    // so we have to use a wrapper.
// [RLVa:KB] - Checked: RLVa-2.0.1
    bool fRlvCanShowName = true;
    if ( (m_fRlvCheck) && (RlvActions::isRlvEnabled()) )
        std::for_each(mUUIDs.begin(), mUUIDs.end(), [&fRlvCanShowName](const LLUUID& idAgent) { fRlvCanShowName &= RlvActions::canShowName(RlvActions::SNC_DEFAULT, idAgent); });

    RlvActions::setShowName(RlvActions::SNC_TELEPORTOFFER, fRlvCanShowName);
    LLAvatarActions::offerTeleport(mUUIDs);
    RlvActions::setShowName(RlvActions::SNC_TELEPORTOFFER, true);
// [/RLVa:KB]
//  LLAvatarActions::offerTeleport(mUUIDs);
}

void PeopleContextMenu::startConference()
{
    uuid_vec_t uuids;
    for (uuid_vec_t::const_iterator it = mUUIDs.begin(); it != mUUIDs.end(); ++it)
    {
        if(*it != gAgentID)
        {
            uuids.push_back(*it);
        }
    }
    LLAvatarActions::startConference(uuids);
}

//== NearbyPeopleContextMenu ===============================================================

void NearbyPeopleContextMenu::buildContextMenu(class LLMenuGL& menu, U32 flags)
{
    menuentry_vec_t items;
    menuentry_vec_t disabled_items;

// [RLVa:KB] - Checked: RLVa-1.5.0
    bool fRlvCanShowName = true;
    if ( (m_fRlvCheck) && (RlvActions::isRlvEnabled()) )
        std::for_each(mUUIDs.begin(), mUUIDs.end(), [&fRlvCanShowName](const LLUUID& idAgent) { fRlvCanShowName &= RlvActions::canShowName(RlvActions::SNC_DEFAULT, idAgent); });

    if (!fRlvCanShowName)
    {
        if (flags & ITEM_IN_MULTI_SELECTION)
        {
            items.push_back(std::string("offer_teleport"));
        }
        else
        {
            items.push_back(std::string("offer_teleport"));
            items.push_back(std::string("request_teleport"));
            items.push_back(std::string("separator_invite_to_group"));
            items.push_back(std::string("zoom_in"));
            items.push_back(std::string("separator_utils"));
            items.push_back(std::string("manage_menu"));
            items.push_back(std::string("block_unblock"));
            items.push_back(std::string("report_abuse"));
        }
    }
    else if (flags & ITEM_IN_MULTI_SELECTION)
// [/RLVa:KB]
//  if (flags & ITEM_IN_MULTI_SELECTION)
    {
        items.push_back(std::string("add_friends"));
        items.push_back(std::string("remove_friends"));
        items.push_back(std::string("im"));
        items.push_back(std::string("call"));
        items.push_back(std::string("share"));
        items.push_back(std::string("pay"));
        items.push_back(std::string("offer_teleport"));
        items.push_back(std::string("separator_utils"));
        bool can_freeze = ALAvatarActions::canFreezeEject(mUUIDs);
        bool can_em = ALAvatarActions::canManageAvatarsEstate(mUUIDs);
        if (can_freeze || can_em)
        {
            items.push_back(std::string("manage_menu"));
            if (can_freeze)
            {
                items.push_back(std::string("freeze"));
                items.push_back(std::string("eject"));
            }
            if (can_em)
            {
                items.push_back(std::string("teleporthome"));
                items.push_back(std::string("estatekick"));
                items.push_back(std::string("estateban"));
            }
        }
        items.push_back(std::string("utils_menu"));
        items.push_back(std::string("copy_full_name"));
        items.push_back(std::string("copy_display_name"));
        items.push_back(std::string("copy_username"));
        items.push_back(std::string("copy_slurl"));
        items.push_back(std::string("copy_uuid"));
    }
    else
    {
        items.push_back(std::string("view_profile"));
        items.push_back(std::string("im"));
        items.push_back(std::string("offer_teleport"));
        items.push_back(std::string("request_teleport"));
        items.push_back(std::string("voice_call"));
        items.push_back(std::string("chat_history"));
        items.push_back(std::string("separator_chat_history"));
        items.push_back(std::string("add_friend"));
        items.push_back(std::string("remove_friend"));
        items.push_back(std::string("invite_to_group"));
        items.push_back(std::string("separator_invite_to_group"));
        items.push_back(std::string("zoom_in"));
        items.push_back(std::string("teleport_to"));
        items.push_back(std::string("map"));
        items.push_back(std::string("share"));
        items.push_back(std::string("pay"));
        items.push_back(std::string("separator_utils"));
        items.push_back(std::string("manage_menu"));
        items.push_back(std::string("block_unblock"));
        items.push_back(std::string("report_abuse"));
        bool can_freeze = ALAvatarActions::canFreezeEject(mUUIDs);
        bool can_em = ALAvatarActions::canManageAvatarsEstate(mUUIDs);
        if (can_freeze || can_em)
        {
            if (can_freeze)
            {
                items.push_back(std::string("freeze"));
                items.push_back(std::string("eject"));
            }
            if (can_em)
            {
                items.push_back(std::string("teleporthome"));
                items.push_back(std::string("estatekick"));
                items.push_back(std::string("estateban"));
            }
        }
        items.push_back(std::string("utils_menu"));
        items.push_back(std::string("copy_full_name"));
        items.push_back(std::string("copy_display_name"));
        items.push_back(std::string("copy_username"));
        items.push_back(std::string("copy_slurl"));
        items.push_back(std::string("copy_uuid"));
    }

    hide_context_entries(menu, items, disabled_items);
}

} // namespace LLPanelPeopleMenus
