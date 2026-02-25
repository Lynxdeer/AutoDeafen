#pragma once

#include <chrono>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>


#include "ipc.h"

#include <Geode/Geode.hpp>
using namespace geode::prelude;

namespace helpers {


	inline std::string getClipboardText() {
		if (!OpenClipboard(nullptr)) return {};
		auto h = GetClipboardData(CF_UNICODETEXT);
		auto p = h ? static_cast<LPCWSTR>(GlobalLock(h)) : nullptr;

		std::string out;
		if (p) {
			int n = WideCharToMultiByte(CP_UTF8, 0, p, -1, nullptr, 0, nullptr, nullptr);
			out.resize(n ? n - 1 : 0);
			if (n) WideCharToMultiByte(CP_UTF8, 0, p, -1, out.data(), n, nullptr, nullptr);
			GlobalUnlock(h);
		}

		CloseClipboard();
		return out;
	}

	inline short getLevelType(GJGameLevel* level) {
		if (level -> m_levelType != GJLevelType::Saved) return 1;
		if (level -> m_dailyID > 0) return 2;
		if (level -> m_gauntletLevel) return 3;

		return 0;
	}

	inline void initIPC() {
		log::info("attempting to initialize ipc");
		std::thread([]() {
			ipc::initializeDiscordAuth();
		}).detach();
	}

	inline long currentTime() { // todo find out whether geode has a better utility for this
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}


	// todo find better variable name

	// std::function<void(web::WebResponse)>
	// <lambda void(web::WebResponse res)>

	inline std::function<void(web::WebResponse)> webHandler = [] (web::WebResponse res) {

		log::info("1");

		if (!res.ok()) {
			log::info("res not ok {} {}", res.code(), res.error());
			return;
		}
		log::info("2");

		// todo I think I could improve this later but this fix works ifne for now
		DISCORD_ACCESS_TOKEN = res.json().unwrapOr("{ access_token: \"\" }")["access_token"].asString().unwrapOr("");
		DISCORD_REFRESH_TOKEN = res.json().unwrapOr("{ refresh_token: \"\" }")["refresh_token"].asString().unwrapOr("");
		TOKEN_EXPIRY = res.json().unwrapOr("{ expires_in: 0 }")["expires_in"].asInt().unwrapOr(0) + currentTime();

		Mod::get()->setSavedValue<std::string>("DISCORD_ACCESS_TOKEN", DISCORD_ACCESS_TOKEN);
		Mod::get()->setSavedValue<std::string>("DISCORD_REFRESH_TOKEN", DISCORD_REFRESH_TOKEN);
		Mod::get()->setSavedValue<long long>("TOKEN_EXPIRY", TOKEN_EXPIRY);

		log::info("got access token: {}", DISCORD_ACCESS_TOKEN);
		log::info("got refresh token: {}", DISCORD_REFRESH_TOKEN);
		log::info("expires {}", TOKEN_EXPIRY);
		initIPC();
	};

	inline void sendRefreshRequest() {

		static TaskHolder<web::WebResponse> listener;

		std::string params =
				   "client_id=" + CLIENT_ID +
				   "&client_secret=" + CLIENT_SECRET +
				   "&grant_type=refresh_token"
				   "&code=" + DISCORD_REFRESH_TOKEN +
				   "&redirect_uri=http://localhost:8000";

		auto req = web::WebRequest();
		req.header("Content-Type", "application/x-www-form-urlencoded");
		req.body(std::vector<uint8_t>(params.begin(), params.end()));

		listener.spawn(
		   req.post("https://discord.com/api/oauth2/token"),
		   webHandler
		);

	}

}
