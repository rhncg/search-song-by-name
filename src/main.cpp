#include "getID.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/MDPopup.hpp>
#include <Geode/utils/web.hpp>
#include <algorithm>
#include <functional>
#include <string>

using namespace geode::prelude;

#include <Geode/modify/CustomSongLayer.hpp>
class $modify(ModCustomSongLayer, CustomSongLayer) {
    struct Fields {
        TaskHolder<web::WebResponse> m_listener;
    };

    bool init(CustomSongDelegate *delegate) {
        if (!CustomSongLayer::init(delegate))
            return false;

        if (this->m_songIDInput) {
            this->m_songIDInput->setAllowedChars(
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
                " ");
            this->m_songIDInput->setMaxLabelLength(200);

            if (auto children = this->m_songIDInput->getChildren()) {
                for (int i = 0; i < children->count(); ++i) {
                    if (auto label = typeinfo_cast<CCLabelBMFont *>(
                            children->objectAtIndex(i))) {
                        if (std::string(label->getString()).find("Enter") !=
                            std::string::npos) {
                            label->setString("Song Name or ID");
                            break;
                        }
                    }
                }
            }

            this->m_songIDInput->refreshLabel();
        }

        return true;
    }

    void textChanged(CCTextInputNode *node) {
        CustomSongLayer::textChanged(node);

        if (node && node == this->m_songIDInput && node->getString().empty()) {
            if (auto children = node->getChildren()) {
                for (int i = 0; i < children->count(); ++i) {
                    if (auto label = typeinfo_cast<CCLabelBMFont *>(
                            children->objectAtIndex(i))) {
                        if (std::string(label->getString()).find("Enter") !=
                            std::string::npos) {
                            label->setString("Song Name or ID");
                            node->refreshLabel();
                            break;
                        }
                    }
                }
            }
        }
    }

    void onSearch(CCObject *sender) {
        auto songInput = this->m_songIDInput;
        if (!songInput) {
            CustomSongLayer::onSearch(sender);
            return;
        }

        std::string searchString = songInput->getString();

        bool isID =
            !searchString.empty() &&
            std::all_of(searchString.begin(), searchString.end(), ::isdigit);

        if (isID) {
            CustomSongLayer::onSearch(sender);
        } else {
            WeakRef<CustomSongLayer> weakSelf = this;

            getIDfromName(
                searchString, m_fields->m_listener,
                [weakSelf](std::string foundID) {
                    auto self = weakSelf.lock();
                    if (!self)
                        return;

                    if (!foundID.empty()) {
                        if (self->m_songIDInput) {
                            self->m_songIDInput->setString(foundID);
                            self->onSearch(nullptr);
                        }
                    } else {
                        createQuickPopup(
                            "No Results",
                            "No songs matched your search or "
                            "Newgrounds might think you're a "
                            "bot. Visit Newgrounds to "
                            "resolve this.",
                            "OK", "Open Newgrounds", [](auto, bool btn2) {
                                if (btn2) {
                                    web::openLinkInBrowser("https://"
                                                           "www.newgrounds."
                                                           "com/");
                                }
                            });
                    }
                });
        }
    }
};

#include <Geode/modify/MoreSearchLayer.hpp>
class $modify(ModMoreSearchLayer, MoreSearchLayer) {
    struct Fields {
        TaskHolder<web::WebResponse> m_listener;
    };

    bool init() {
        if (!MoreSearchLayer::init())
            return false;

        if (this->m_enterSongID) {
            this->m_enterSongID->setAllowedChars(
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
                " ");
            this->m_enterSongID->setMaxLabelLength(200);

            if (auto children = this->m_enterSongID->getChildren()) {
                for (int i = 0; i < children->count(); ++i) {
                    if (auto label = typeinfo_cast<CCLabelBMFont *>(
                            children->objectAtIndex(i))) {
                        if (std::string(label->getString()).find("Enter") !=
                            std::string::npos) {
                            label->setString("Song Name or ID");
                            break;
                        }
                    }
                }
            }

            this->m_enterSongID->refreshLabel();
        }

        auto searchSpr =
            CCSprite::createWithSpriteFrameName("gj_findBtn_001.png");

        auto searchBtn = CCMenuItemSpriteExtra::create(
            searchSpr, this, menu_selector(ModMoreSearchLayer::onSearch));

        auto menu = CCMenu::create();
        menu->addChild(searchBtn);
        menu->setPosition(m_enterSongID->getPosition() + ccp(-110.f, 0.f));

        menu->setID("song-name-search-menu"_spr);
        menu->setVisible(false);

        if (this->m_customSongNodes) {
            this->m_customSongNodes->addObject(menu);
        }

        if (this->m_mainLayer) {
            this->m_mainLayer->addChild(menu);
        } else {
            this->addChild(menu);
        }

        this->updateLayout();

        return true;
    }

    void toggleSongNodes(bool custom, bool disable) {
        MoreSearchLayer::toggleSongNodes(custom, disable);

        if (auto menu =
                this->m_mainLayer->getChildByID("song-name-search-menu"_spr)) {
            menu->setVisible(custom && !disable);
        }

        if (custom && !disable && this->m_enterSongID) {
            this->m_enterSongID->setAllowedChars(
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
                " ");
            this->m_enterSongID->setMaxLabelLength(200);

            if (auto children = this->m_enterSongID->getChildren()) {
                for (int i = 0; i < children->count(); ++i) {
                    if (auto label = typeinfo_cast<CCLabelBMFont *>(
                            children->objectAtIndex(i))) {
                        if (std::string(label->getString()).find("Enter") !=
                            std::string::npos) {
                            label->setString("Song Name or ID");
                            break;
                        }
                    }
                }
            }

            this->m_enterSongID->refreshLabel();
        }
    }

    void onSearch(CCObject *sender) {
        std::string searchString = m_enterSongID->getString();
        if (searchString == "")
            return;

        bool isID =
            !searchString.empty() &&
            std::all_of(searchString.begin(), searchString.end(), ::isdigit);

        if (!isID) {
            WeakRef<MoreSearchLayer> weakSelf = this;

            getIDfromName(
                searchString, m_fields->m_listener,
                [weakSelf](std::string foundID) {
                    auto self = weakSelf.lock();
                    if (!self)
                        return;
                    if (!foundID.empty()) {
                        if (self->m_enterSongID) {
                            self->m_enterSongID->setString(foundID);
                        }
                    } else {
                        createQuickPopup(
                            "No Results",
                            "No songs matched your search or "
                            "Newgrounds might think you're a "
                            "bot. Visit Newgrounds to "
                            "resolve this.",
                            "OK", "Open Newgrounds", [](auto, bool btn2) {
                                if (btn2) {
                                    web::openLinkInBrowser("https://"
                                                           "www.newgrounds."
                                                           "com/");
                                }
                            });
                    }
                });
        }
    }
};