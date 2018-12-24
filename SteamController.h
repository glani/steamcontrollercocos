//
// Created by denis on 12/19/18.
//

#ifndef CCWARIO_STEAMCONTROLLER_H
#define CCWARIO_STEAMCONTROLLER_H

#include "platform/CCPlatformMacros.h"
#include <string>
#include <vector>
#include <unordered_map>

#include "SteamControllerCommons.h"

NS_CC_BEGIN
class SteamControllerImpl;
class EventListenerSteamController;
class EventSteamController;
class EventDispatcher;


class SteamController {
public:
    static const int TAG_UNSET = -1;
    static const char* DEVICE_UNSET;

    static void startDiscoverySteamController();

    static void stopDiscoverySteamController();

    static SteamController* getControllerByDeviceId(const std::string& deviceId);

    static SteamController* getControllerByTag(int tag);

    const std::string& getDeviceId() const;

    int getTag() const;

    void setTag(int tag);

	static void exit();

private:
    std::string deviceId;
    int tag;

    EventDispatcher* eventDispatcher;
    EventSteamController *connectEvent;
    EventSteamController *updateEvent;

    SteamController();
    virtual ~SteamController();

    void init();

    void onConnected();
    void onDisconnected();

    friend class SteamControllerImpl;
    friend class EventListenerSteamController;

    void onUpdated(SteamControllerValues &values);
};

NS_CC_END
#endif //CCWARIO_STEAMCONTROLLER_H
