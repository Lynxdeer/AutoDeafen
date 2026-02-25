#pragma once

#include <windows.h>
#include <shellapi.h>

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Popup.hpp>

/**
 * To anyone reading my code- I initially made this mod with the intention to have my own application interface with the discord client.
 * However, after finding out that the mod wasn't working on other peoples' machines, I discovered that RPC has been in a "private beta" for 7 years.
 * Therefore, I have to have everyone create their own application. Kinda annoying but still better than keybinds.
 */
class AuthLayer : public geode::Popup {
public:
    void onClose(CCObject* a) override {
        Popup::onClose(a);
        currentlyInMenu = false;
    }
    static AuthLayer* create() {
        auto ret = new AuthLayer;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
protected:
    CCLabelBMFont* clientId;
    CCLabelBMFont* clientSecret;
    void openTutorial(CCObject*) {
        ShellExecuteA(nullptr, "open", "https://lynxdeer.xyz/autodeafen_setup.html", nullptr, nullptr, SW_SHOWNORMAL);
        log::info("opened docs");
    }
    void pasteClientId(CCObject*) {

        std::string paste = helpers::getClipboardText();
        log::info("pasted client id {}", paste);

        CLIENT_ID = paste;
        clientId->setString(paste.c_str());

    }
    void pasteClientSecret(CCObject*) {

        std::string paste = helpers::getClipboardText();
        log::info("pasted client secret {}", paste);

        CLIENT_SECRET = paste;
        clientSecret->setString(paste.c_str());

    }
    void done(CCObject* c) {

        log::info("next");
        oauth::startServer();
        ShellExecuteA(nullptr, "open", ("https://discord.com/oauth2/authorize?client_id=" + CLIENT_ID + "&response_type=code&redirect_uri=http%3A%2F%2Flocalhost%3A8000&scope=rpc.voice.write+rpc").c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        AuthLayer::onClose(c);


    }
    bool init() override {
        if (!geode::Popup::init(300, 240)) return false;
        currentlyInMenu = true;

        CCPoint topMiddle = ccp(m_size.width/2.f, m_size.height);

        auto topLabel = CCLabelBMFont::create("Setup", "goldFont.fnt");
        topLabel->setAnchorPoint({0.5, 0.5});
        topLabel->setScale(1.0f);
        topLabel->setPosition(topMiddle + ccp(0, 5));

        clientId = CCLabelBMFont::create("...", "chatFont-uhd.fnt");
        clientId->setAnchorPoint({0.5, 0.5});
        clientId->setScale(0.5f);
        clientId->setPosition(topMiddle + ccp(0, -60));

        clientSecret = CCLabelBMFont::create("...", "chatFont-uhd.fnt");
        clientSecret->setAnchorPoint({0.5, 0.5});
        clientSecret->setScale(0.5f);
        clientSecret->setPosition(topMiddle + ccp(0, -120));

        auto tutorialButton = CCMenuItemSpriteExtra::create(ButtonSprite::create("Open Tutorial"), this, menu_selector(AuthLayer::openTutorial));
        tutorialButton->setAnchorPoint({0.5, 0.5});
        tutorialButton->setPosition(topMiddle + ccp(0, -30));

        auto pasteClientIdButton = CCMenuItemSpriteExtra::create(ButtonSprite::create("Paste Client ID"), this, menu_selector(AuthLayer::pasteClientId));
        pasteClientIdButton->setAnchorPoint({0.5, 0.5});
        pasteClientIdButton->setPosition(topMiddle + ccp(0, -90));

        auto pasteClientSecretButton = CCMenuItemSpriteExtra::create(ButtonSprite::create("Paste Client Secret"), this, menu_selector(AuthLayer::pasteClientSecret));
        pasteClientSecretButton->setAnchorPoint({0.5, 0.5});
        pasteClientSecretButton->setPosition(topMiddle + ccp(0, -150));


        auto doneButton = CCMenuItemSpriteExtra::create(ButtonSprite::create("Next"), this, menu_selector(AuthLayer::done));
        doneButton->setAnchorPoint({0.5, 0.5});
        doneButton->setPosition(topMiddle + ccp(0, -200));

        auto menu = CCMenu::create();
        menu -> setPosition( {0, 0} );

        menu->addChild(topLabel);
        menu->addChild(clientId);
        menu->addChild(clientSecret);
        menu->addChild(tutorialButton);
        menu->addChild(pasteClientIdButton);
        menu->addChild(pasteClientSecretButton);
        menu->addChild(doneButton);

        m_mainLayer -> addChild(menu);

        return true;
    }
};

namespace gui {

    // todo find better name
    inline void openPastingSetup() {
        AuthLayer::create()->show();
    }

    inline void openSetupPopup() {

        oauth::startServer();
        geode::createQuickPopup(
            "AutoDeafen",
            "<cj>AutoDeafen requires some setting-up.</c> <cg>Press 'Open' to open a tutorial link on how to set up autodeafen.</c>",
            "Later", "Open",
            [](auto, bool btn2) {
                if (btn2) {
                    log::info("opening docs");
                    openPastingSetup();
                    ShellExecuteA(nullptr, "open", "https://lynxdeer.xyz/autodeafen_setup.html", nullptr, nullptr, SW_SHOWNORMAL);
                    log::info("opened docs");
                }
            }
        );


    }
    
}


// this has to go outside of the namespace because otherwise msvc kills itself
class ConfigLayer : public geode::Popup {
    public:
        TextInput* percentageInput;
        void runSetup(CCObject*) {
            gui::openSetupPopup();
        }
        void toggleEnabled(CCObject*) {
            DEAFEN_ENABLED = !DEAFEN_ENABLED;
        }
        void onClose(CCObject* a) override {
            Popup::onClose(a);
            if (percentageInput != nullptr && !percentageInput->getString().empty())
                DEAFEN_PERCENTAGE = geode::utils::numFromString<int>(percentageInput -> getString()).ok().value_or(Mod::get()->getSettingValue<int>("default_percentage"));
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
                percentageInput->setWidth(40);
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
                reauthLabel = CCLabelBMFont::create("Authentication Failed! Press the Re-Setup button to try again.", "bigFont.fnt");
                // keybindLabel = CCLabelBMFont::create("To use the mod, press the \n<cg>Edit Keybind</c> button\nand press your \ndiscord <co>Toggle Deafen</c> keybind \nset in \n<cb>Discord Settings</c> > <cp>Keybinds</c>", "chatFont-uhd.fnt");
                reauthLabel->setAnchorPoint({0.5, 0});
                reauthLabel->setScale(0.4f);
                reauthLabel->setAlignment(kCCTextAlignmentCenter);
                reauthLabel->setPosition(topMiddle + ccp(0, -100));
            }

            auto reAuthButton = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Re-Setup"),
                this,
                menu_selector(ConfigLayer::runSetup)
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

    // this a memory leak I think but its not significant (todo: test)
    ConfigLayer::create()->show();

}