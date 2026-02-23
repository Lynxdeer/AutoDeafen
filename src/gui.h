#pragma once

#include <windows.h>
#include <shellapi.h>

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Popup.hpp>



namespace gui {
    
inline void openAuthPopup() {
    
    oauth::startServer();
    geode::createQuickPopup(
        "AutoDeafen",
        "<cj>The mod needs to get permissions from your discord account to work.</c> <cg>Press 'Open' to open a link to give permissions.</c>",
        "Later", "Open",
        [](auto, bool btn2) {
            if (btn2) {
                log::info("opening authorization link");
                // can add rpc.voice.read if needed later but for now it is not
                ShellExecuteA(nullptr, "open", "https://discord.com/oauth2/authorize?client_id=1289578567151390793&response_type=code&redirect_uri=http%3A%2F%2Flocalhost%3A8000&scope=rpc.voice.write+rpc", nullptr, nullptr, SW_SHOWNORMAL);
            }
        }
    );
    
}
    
}


// this has to go outside of the namespace because otherwise msvc kills itself
class ConfigLayer : public geode::Popup {
    public:
        TextInput* percentageInput;
        void reAuthenticate(CCObject*) {
            gui::openAuthPopup();
        }
        void toggleEnabled(CCObject*) {
            DEAFEN_ENABLED = !DEAFEN_ENABLED;
        }
        void onClose(CCObject* a) override {
            Popup::onClose(a);
            if (percentageInput != nullptr && !percentageInput->getString().empty())
                DEAFEN_PERCENTAGE = stoi(percentageInput -> getString());
            currentlyInMenu = false;
        }
        static ConfigLayer* create() {
            auto ret = new ConfigLayer;
            if (ret && ret->init()) {
                ret->autorelease();
                return ret;
            }
            delete ret;
            return nullptr;
        }
    protected:
        bool init() override {

            if (!geode::Popup::init(300, 200)) return false;

            currentlyInMenu = true;

            this->setKeyboardEnabled(true);

            // auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

            CCPoint topLeftCorner = ccp(0, m_size.height);
            CCPoint topMiddle = ccp(m_size.width/2.f, m_size.height);

            auto topLabel = CCLabelBMFont::create("AutoDeafen", "goldFont.fnt");
            topLabel->setAnchorPoint({0.5, 0.5});
            topLabel->setScale(1.0f);
            topLabel->setPosition(topLeftCorner + ccp(142, 5));

            CCLabelBMFont* enabledLabel;
            CCLabelBMFont* percentageLabel;
            CCLabelBMFont* reauthLabel;

            CCMenuItemToggler* enabledButton;
            if (ipc::authenticated) {
                enabledLabel = CCLabelBMFont::create("Enabled", "bigFont.fnt");
                enabledLabel->setAnchorPoint({0, 0.5});
                enabledLabel->setScale(0.7f);
                enabledLabel->setPosition(topLeftCorner + ccp(60, -60)); // 80 = center

                enabledButton = CCMenuItemToggler::create(
                    CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
                    CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
                    this,
                    menu_selector(ConfigLayer::toggleEnabled)
                );

                enabledButton->setPosition(enabledLabel->getPosition() + ccp(140,0));
                enabledButton->setScale(0.85f);
                enabledButton->setClickable(true);
                enabledButton->toggle(DEAFEN_ENABLED);

                percentageInput = TextInput::create(100.f, "%");

                percentageInput->setFilter("0123456789");
                percentageInput->setPosition(enabledButton->getPosition() + ccp(0, -40));
                percentageInput->setScale(0.85f);
                percentageInput->setMaxCharCount(2);
                percentageInput->setEnabled(true);
                percentageInput->setString(std::to_string(DEAFEN_PERCENTAGE));

                percentageLabel = CCLabelBMFont::create("Percent", "bigFont.fnt");
                percentageLabel->setAnchorPoint({0, 0.5});
                percentageLabel->setScale(0.7f);
                percentageLabel->setPosition(topLeftCorner + ccp(60, -100));
            } else {
                reauthLabel = CCLabelBMFont::create("Authentication Failed! Press the Re-Authenticate button to try again.", "bigFont.fnt");
                // keybindLabel = CCLabelBMFont::create("To use the mod, press the \n<cg>Edit Keybind</c> button\nand press your \ndiscord <co>Toggle Deafen</c> keybind \nset in \n<cb>Discord Settings</c> > <cp>Keybinds</c>", "chatFont-uhd.fnt");
                reauthLabel->setAnchorPoint({0.5, 0});
                reauthLabel->setScale(0.4f);
                reauthLabel->setAlignment(kCCTextAlignmentCenter);
                reauthLabel->setPosition(topMiddle + ccp(0, -100));
            }

            auto reAuthButton = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Re-Authenticate"),
                this,
                menu_selector(ConfigLayer::reAuthenticate)
            );
            reAuthButton->setAnchorPoint({0.5, 0.5});
            reAuthButton->setPosition(topLeftCorner + ccp(142, -150));

            // auto debugButton = CCMenuItemSpriteExtra::create(
            // 	ButtonSprite::create("Debug"),
            // 	this,
            // 	menu_selector(ConfigLayer::runDebugs)
            // );
            // debugButton->setAnchorPoint({0.5, 0.5});
            // debugButton->setPosition(topLeftCorner + ccp(142, -175));

            auto menu = CCMenu::create();
            menu -> setPosition( {0, 0} );

            if (ipc::authenticated) {
                menu -> addChild(enabledButton);
                menu -> addChild(percentageInput);
            }

            menu -> addChild(reAuthButton);
            // menu -> addChild(debugButton);
            m_mainLayer -> addChild(topLabel);
            if (ipc::authenticated) {
                m_mainLayer -> addChild(enabledLabel);
                m_mainLayer -> addChild(percentageLabel);
            } else {
                m_mainLayer -> addChild(reauthLabel);
            }

            m_mainLayer -> addChild(menu);

            return true;
        }
};


inline void openModPopup() {

    auto layer = ConfigLayer::create();
    layer->show();

}