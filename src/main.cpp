
std::string CLIENT_ID = "";
std::string CLIENT_SECRET = "";

std::string DISCORD_ACCESS_TOKEN;
std::string DISCORD_REFRESH_TOKEN;
long long TOKEN_EXPIRY = 0;

std::string CURRENT_LEVEL;
bool DEAFEN_ENABLED = false;
int DEAFEN_PERCENTAGE = 50;
bool deafenedThisAttempt = false;
bool hasDied = false;

bool currentlyInMenu = false;

#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include "ipc.h"
#include "helpers.h"
#include "oauth.h"
#include "gui.h"

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <cocos2d.h>

$on_mod(Loaded) {
    if (Mod::get()->hasSavedValue("DISCORD_ACCESS_TOKEN")) {

        if (!Mod::get()->hasSavedValue("DISCORD_REFRESH_TOKEN") || !Mod::get()->hasSavedValue("TOKEN_EXPIRY")) return;

        DISCORD_REFRESH_TOKEN = Mod::get()->getSavedValue<std::string>("DISCORD_REFRESH_TOKEN");
        TOKEN_EXPIRY = Mod::get()->getSavedValue<long long>("TOKEN_EXPIRY");

        if (helpers::currentTime() > TOKEN_EXPIRY) {
            log::info("sending refresh request");
            helpers::sendRefreshRequest();
        } else {
            DISCORD_ACCESS_TOKEN = Mod::get()->getSavedValue<std::string>("DISCORD_ACCESS_TOKEN");
            log::info("loaded saved auth token {}", DISCORD_ACCESS_TOKEN);
            helpers::initIPC();
        }
    }
};

class $modify(PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    	deafenedThisAttempt = false;
    	hasDied = false;

        int id = m_level -> m_levelID.value();
		short levelType = helpers::getLevelType(level);
		if (levelType == 1) id = m_level -> m_M_ID;

    	CURRENT_LEVEL = std::to_string(id) + "-" + std::to_string(levelType);

    	bool defaultEnabled = Mod::get()->getSettingValue<int>("default_enabled");
    	int defaultDeafenPercentage = Mod::get()->getSettingValue<int>("default_percentage");

    	if (Mod::get()->hasSavedValue(CURRENT_LEVEL)) {
    		auto value = Mod::get()->getSavedValue<matjson::Value>(CURRENT_LEVEL);
    		DEAFEN_ENABLED = value["e"].asBool().ok().value_or(defaultEnabled);
    		DEAFEN_PERCENTAGE = value["p"].asInt().ok().value_or(defaultDeafenPercentage);
    	} else {
    		DEAFEN_ENABLED = defaultEnabled;
    		DEAFEN_PERCENTAGE = defaultDeafenPercentage;
    	}
  
        return true;
    }

    void postUpdate(float p0) {
		PlayLayer::postUpdate(p0);
		if (this->m_isPracticeMode && !Mod::get()->getSettingValue<bool>("practice")) { return; }
    	if (!DEAFEN_ENABLED) return;
    	if (this->m_hasCompletedLevel) {
    		if (deafenedThisAttempt) {
    			ipc::deafen(false);
    			deafenedThisAttempt = false;
    			hasDied = false;
    		}
    		return;
    	}

		if (getCurrentPercentInt() >= DEAFEN_PERCENTAGE && !deafenedThisAttempt) {
			ipc::deafen(true);
            deafenedThisAttempt = true; // todo: find out if quitting the lvl breaks this
		}
	}
    void resetLevel() {
    	PlayLayer::resetLevel();
    	// if (deafenedThisAttempt) {
    	deafenedThisAttempt = false;
    	hasDied = false;
    	// }
	}

};

class $modify(PlayerObject) {
    void playerDestroyed(bool p0) {
        auto playLayer = PlayLayer::get();
        if (playLayer && playLayer->m_level && this == playLayer->m_player1 && !playLayer->m_level->isPlatformer()) {
            if (deafenedThisAttempt && !hasDied) {
                ipc::deafen(false);
                hasDied = true;
            }
        }
        PlayerObject::playerDestroyed(p0);
    }
};

class $modify(MyPauseLayer, PauseLayer) {

	void onQuit(cocos2d::CCObject* sender) {
		PauseLayer::onQuit(sender);
		if (deafenedThisAttempt) {
			ipc::deafen(false);
		}
		deafenedThisAttempt = false;
		hasDied = false;

		bool defaultEnabled = Mod::get()->getSettingValue<int>("default_enabled");
		int defaultDeafenPercentage = Mod::get()->getSettingValue<int>("default_percentage");

		if (!(DEAFEN_ENABLED == defaultEnabled && defaultDeafenPercentage == DEAFEN_PERCENTAGE)) {

			// not the most readable but nobody's reading it so idc
			auto json = matjson::Value();
			json["e"] = DEAFEN_ENABLED;
			json["p"] = DEAFEN_PERCENTAGE;

			Mod::get()->setSavedValue(CURRENT_LEVEL, json);

		}

	}

	void onAutoDeafenMenuClick(CCObject* target) {
		if (ipc::authenticated) {
			openModPopup();
		} else {
			gui::openSetupPopup();
		}
	}
	void customSetup() {
        PauseLayer::customSetup();
        auto menu = this->getChildByID("right-button-menu");
        
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_musicOffBtn_001.png");
        auto btn = CCMenuItemSpriteExtra::create(sprite, sprite, this, menu_selector(MyPauseLayer::onAutoDeafenMenuClick));
        // auto btn = CCMenuItemExt::createSpriteExtra(
        //     sprite,
        //     [this](CCMenuItemSpriteExtra* btn) {
        //

        //     }
        // );
        menu->addChild(btn);
        menu->updateLayout();
    }
};