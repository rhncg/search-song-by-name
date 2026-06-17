#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <algorithm>
#include <Geode/modify/CustomSongLayer.hpp>
#include <string>

using namespace geode::prelude;

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
            std::string query = searchString;
            std::replace(query.begin(), query.end(), ' ', '+');

            std::string url = "https://www.newgrounds.com/search/"
                              "summary?suitabilities=etm&terms=" +
                              query;
            auto req = web::WebRequest();

            WeakRef<CustomSongLayer> weakSelf = this;

            m_fields->m_listener.spawn(
                req.get(url), [weakSelf](web::WebResponse const &res) {
                    auto self = weakSelf.lock();
                    if (!self)
                        return;

                    if (res.ok()) {
                        auto const &rawBytes = res.data();
                        std::string target =
                            "https://www.newgrounds.com/audio/listen/";

                        auto it = std::search(rawBytes.begin(), rawBytes.end(),
                                              target.begin(), target.end());

                        if (it != rawBytes.end()) {
                            auto idStart = it + target.length();
                            std::string foundID = "";

                            while (idStart != rawBytes.end() &&
                                   std::isdigit(*idStart)) {
                                foundID += static_cast<char>(*idStart);
                                idStart++;
                            }

                            if (!foundID.empty() && self->m_songIDInput) {
                                self->m_songIDInput->setString(foundID.c_str());
                                self->onSearch(nullptr);
                                return;
                            }
                        }

                        auto alert = FLAlertLayer::create(
                            "No Results", "No songs matched your search.",
                            "OK");
                        if (alert)
                            alert->show();

                    } else {
                        auto errorAlert = FLAlertLayer::create(
                            "Error", "Failed to connect to Newgrounds. Newgrounds might be blocking the request or your internet connection might be down.", "OK");
                        if (errorAlert)
                            errorAlert->show();
                    }
                });
        }
    }
};