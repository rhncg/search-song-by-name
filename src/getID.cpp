#include "getID.hpp"

void getIDfromName(const std::string &name,
                   TaskHolder<web::WebResponse> &taskHolder,
                   std::function<void(std::string)> callback) {
    std::string query = name;
    std::replace(query.begin(), query.end(), ' ', '+');

    std::string url =
        "https://www.newgrounds.com/search/summary?suitabilities=etm&terms=" +
        query;
    auto req = web::WebRequest();

    taskHolder.spawn(req.get(url), [callback](web::WebResponse const &res) {
        if (!res.ok()) {
            callback("");
            return;
        }

        auto const &rawBytes = res.data();
        std::string target = "https://www.newgrounds.com/audio/listen/";

        auto it = std::search(rawBytes.begin(), rawBytes.end(), target.begin(),
                              target.end());

        if (it != rawBytes.end()) {
            auto idStart = it + target.length();
            std::string foundID = "";

            while (idStart != rawBytes.end() && std::isdigit(*idStart)) {
                foundID += static_cast<char>(*idStart);
                idStart++;
            }

            callback(foundID);
            return;
        }

        callback("");
    });
}