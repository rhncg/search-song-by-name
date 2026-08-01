#pragma once
#include <Geode/utils/web.hpp>
using namespace geode::prelude;

void getIDfromName(const std::string& name, TaskHolder<web::WebResponse>& taskHolder, std::function<void(std::string)> callback);