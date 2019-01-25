//
// Created by denis on 12/19/18.
//

#include "SteamController.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX || CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

#include <functional>
#include <string>
#include <unordered_set>

#include "base/ccMacros.h"
#include "base/CCDirector.h"
#include "base/CCScheduler.h"
#include "base/CCAsyncTaskPool.h"
#include "EventSteamController.h"
#include "SteamControllerCommons.h"

#include "sclib/include/sc.h"

NS_CC_BEGIN
    static const float DISCOVERY_INTERVAL = 1.0f;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    static const size_t TotalEvents = 10;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    static const size_t TotalEvents = 3;
struct SteamControllerEventHolder {
    SteamControllerEvent** events;
    uint8_t* eventsRawData;
    uint8_t totalRead;
    struct SteamControllerEventHolder* asyncLoad;
    SteamControllerDevice *pDevice;
    SteamController * steamController;
};
#endif
    struct SteamControllerSingleEventHolder {
        SteamControllerDevice *pDevice;
        SteamController *pController;
        SteamControllerEvent event;
        int evtType;
    };

    class SteamControllerImpl {
    public:
        static SteamControllerImpl *s_SteamControllerImpl;

        static void destroyInstance() {
            if (s_SteamControllerImpl == nullptr) {
                delete s_SteamControllerImpl;
            }
        }

        static SteamControllerImpl *getInstance() {
            if (s_SteamControllerImpl == nullptr) {
                s_SteamControllerImpl = new(std::nothrow) SteamControllerImpl();
            }
            return s_SteamControllerImpl;
        }

        SteamControllerImpl() : currentTime(0) {
            SteamController_Init();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
            events = new SteamControllerEvent *[TotalEvents];
            size_t i1 = sizeof(SteamControllerEvent);

            eventsRawData = (uint8_t *) malloc(TotalEvents * i1);
            uint8_t *pEventsRawData = eventsRawData;
            for (size_t i = 0; i < TotalEvents; ++i) {
                events[i] = static_cast<SteamControllerEvent *>(static_cast<void *>(pEventsRawData));
                pEventsRawData += i1;
            }
#endif
        }

        virtual ~SteamControllerImpl() {
            stopDiscoverySteamController();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
            // TODO
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
            delete eventsRawData;
            delete[] events;
#endif
            SteamController_Exit();
        }

        bool processEvents(int total_read, SteamControllerEvent **events, SteamControllerDevice *pDevice,
                           SteamController *steamController) {
            bool isDisconnected = false;

            auto eu = SteamControllerSingleEventHolder();
            eu.evtType = 0;

            for (int i = 0; i < total_read; ++i) {
                SteamControllerEvent *event = events[i];
                int res = event->eventType;
                if (res == STEAMCONTROLLER_EVENT_CONNECTION &&
                    event->connection.details == STEAMCONTROLLER_CONNECTION_EVENT_DISCONNECTED) {
                    CCLOG("Device %p is not connected (anymore), trying next one...\n", pDevice);
                    isDisconnected = true;
                    break;
                } else if (res == STEAMCONTROLLER_EVENT_UPDATE) {
                    if (event->update.buttons == 0
                        && event->update.leftXY.x == 0
                        && event->update.leftXY.y == 0) {
                    } else {

                        eu.pController = steamController;
                        eu.pDevice = pDevice;
                        eu.event = *event;
                        eu.evtType = res;
                    }
                }
            }

            if (!isDisconnected && eu.evtType > 0) {
                this->onUpdate(eu.pDevice, eu.pController, eu.event);
            }

            return isDisconnected;
        }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        void onReadEvent(void* data) {
        // correct async reading
        SteamControllerEventHolder* sceh = static_cast<SteamControllerEventHolder*>(data);

        SteamControllerDevice* pDevice = sceh->pDevice;
        if (steamControllerDeviceMap.empty())
        {
            return;
        }
        bool isFound = false;

        SteamController* steamController = nullptr;
        for(const auto& p : steamControllerDeviceMap)
        {
            if(p.second.first == pDevice)
            {
                steamController = p.second.second;
                isFound = true;
                break;
            }
        }
        if (!isFound)
        {
            return;
        }

        const char* _deviceId = SteamController_GetId(pDevice);
        if (_deviceId == nullptr) {
            // already closed
            return;
        }

        const std::string deviceId(_deviceId);

        uint8_t total_read = sceh->totalRead;
        SteamControllerEvent** events = sceh->events;

        auto isDisconnected = processEvents(total_read, events, pDevice, steamController);
        if (isDisconnected) {
            if (_deviceId != nullptr) {
                onDisconnect(deviceId);
            }
        }

        delete sceh->eventsRawData;
        delete[] sceh->events;
        delete sceh;
    }
#endif

        void update(float dt) {
            if (steamControllerDeviceMap.size() == 0) {
                currentTime += dt;
                if (currentTime > DISCOVERY_INTERVAL) {
                    // update once per second
                    currentTime -= DISCOVERY_INTERVAL;
                    startDiscoverySteamController();
                }
            } else {
                std::unordered_set<std::string> disconnected(steamControllerDeviceMap.size());
                for (auto pair : steamControllerDeviceMap) {
                    SteamControllerDevice *pDevice = pair.second.first;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
                    SteamControllerEvent** events = new SteamControllerEvent*[TotalEvents];
                size_t i1 = sizeof(SteamControllerEvent);

                uint8_t* eventsRawData = (uint8_t*)malloc(TotalEvents * i1);
                memset(eventsRawData, 0, TotalEvents * sizeof(SteamControllerEvent));
                uint8_t* pEventsRawData = eventsRawData;
                SteamControllerEventHolder* sceh = new SteamControllerEventHolder();
                sceh->events = events;
                sceh->eventsRawData = eventsRawData;
                sceh->totalRead = 0;
                sceh->pDevice = pDevice;
                sceh->steamController = pair.second.second;
                sceh->asyncLoad = sceh;

                for (size_t i = 0; i < TotalEvents; ++i) {
                    events[i] = static_cast<SteamControllerEvent*>(static_cast<void*>(pEventsRawData));
                    pEventsRawData += i1;
                }

                AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_IO, CC_CALLBACK_1(SteamControllerImpl::onReadEvent, this), sceh, [sceh]()
                {
                    sceh->asyncLoad->totalRead = SteamController_ReadEvents(sceh->pDevice, sceh->asyncLoad->events, TotalEvents);
                });
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
                    SteamControllerEvent **pEvents = events;
                    memset(eventsRawData, 0, TotalEvents * sizeof(SteamControllerEvent));

                    int totalRead = SteamController_ReadEvents(pDevice, pEvents, TotalEvents);
                    auto isDisconnected = processEvents(totalRead, events, pDevice, pair.second.second);
                    if (isDisconnected) {
                        disconnected.emplace(SteamController_GetId(pDevice));
                    }
#endif
#ifdef _WIN32
                    //if (totalRead == 0 &&
                //	false == SteamController_IsWirelessDongle(pDevice)) {
                //	const std::string deviceId(SteamController_GetId(pDevice));
                //	disconnected.emplace(deviceId);
                //	break;
                //}
#endif


                }
                for (auto deviceId : disconnected) {
                    onDisconnect(deviceId);
                }
            }
        }

        void onUpdate(SteamControllerDevice *pDevice, SteamController *pController, const SteamControllerEvent &event) {

            SteamControllerValues v;
            v.ButtonA = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_A) > 0 ? SteamControllerValues::YES
                                                                                         : SteamControllerValues::NO);
            v.ButtonX = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_X) > 0 ? SteamControllerValues::YES
                                                                                         : SteamControllerValues::NO);
            v.ButtonB = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_B) > 0 ? SteamControllerValues::YES
                                                                                         : SteamControllerValues::NO);
            v.ButtonY = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_Y) > 0 ? SteamControllerValues::YES
                                                                                         : SteamControllerValues::NO);
            v.ButtonRT = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_RT) > 0 ? SteamControllerValues::YES
                                                                                           : SteamControllerValues::NO);
            v.ButtonLT = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_LT) > 0 ? SteamControllerValues::YES
                                                                                           : SteamControllerValues::NO);
            v.ButtonRS = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_RS) > 0 ? SteamControllerValues::YES
                                                                                           : SteamControllerValues::NO);
            v.ButtonLS = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_LS) > 0 ? SteamControllerValues::YES
                                                                                           : SteamControllerValues::NO);

            v.ButtonDpadUp = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_DPAD_UP) > 0
                                        ? SteamControllerValues::YES : SteamControllerValues::NO);
            v.ButtonDpadRight = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_DPAD_RIGHT) > 0
                                           ? SteamControllerValues::YES : SteamControllerValues::NO);
            v.ButtonDpadLeft = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_DPAD_LEFT) > 0
                                          ? SteamControllerValues::YES : SteamControllerValues::NO);
            v.ButtonDpadDown = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_DPAD_DOWN) > 0
                                          ? SteamControllerValues::YES : SteamControllerValues::NO);

            v.ButtonPrev = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_PREV) > 0
                                      ? SteamControllerValues::YES : SteamControllerValues::NO);
            v.ButtonHome = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_HOME) > 0
                                      ? SteamControllerValues::YES : SteamControllerValues::NO);
            v.ButtonNext = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_NEXT) > 0
                                      ? SteamControllerValues::YES : SteamControllerValues::NO);

            v.ButtonRGrip = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_LG) > 0
                                       ? SteamControllerValues::YES : SteamControllerValues::NO);
            v.ButtonLGrip = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_RG) > 0
                                       ? SteamControllerValues::YES : SteamControllerValues::NO);

            v.ButtonStick = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_STICK) > 0
                                       ? SteamControllerValues::YES : SteamControllerValues::NO);
            v.ButtonRPad = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_RPAD) > 0
                                      ? SteamControllerValues::YES : SteamControllerValues::NO);

            v.TouchLFinger = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_LFINGER) > 0
                                        ? SteamControllerValues::YES : SteamControllerValues::NO);
            v.TouchRFinger = (uint8_t) ((event.update.buttons & STEAMCONTROLLER_BUTTON_RFINGER) > 0
                                        ? SteamControllerValues::YES : SteamControllerValues::NO);

            v.LeftPadX = 0;
            v.LeftPadY = 0;
            v.StickX = 0;
            v.StickY = 0;

            if (v.TouchLFinger) {
                v.LeftPadX = clampf(event.update.leftXY.x / 32767.f, -1, 1);
                v.LeftPadY = clampf(event.update.leftXY.y / 32767.f, -1, 1);
            } else {

                if ((event.update.buttons & STEAMCONTROLLER_FLAG_PAD_STICK) != 0
                    || (event.update.leftXY.x != 0 || event.update.leftXY.y != 0)) {
                    v.StickX = clampf(event.update.leftXY.x / 32767.f, -1, 1);
                    v.StickY = clampf(event.update.leftXY.y / 32767.f, -1, 1);
                    v.Stick = SteamControllerValues::YES;
                } else {
                    v.Stick = SteamControllerValues::NO;
                }
            }

            v.RightPadX = clampf(event.update.rightXY.x / 32767.f, -1, 1);
            v.RightPadY = clampf(event.update.rightXY.y / 32767.f, -1, 1);

            v.LeftTrigger = clampf(event.update.leftTrigger / 255.f, -1, 1);
            v.RightTrigger = clampf(event.update.rightTrigger / 255.f, -1, 1);

            pController->onUpdated(v);
        }

        void startDiscoverySteamController() {
            struct SteamControllerDeviceEnum *pEnum = SteamController_EnumControllerDevices();
            while (pEnum) {
                SteamControllerEvent event;
                SteamControllerDevice *pDevice = SteamController_Open(pEnum);
                if (pDevice) {
                    std::string deviceId(SteamController_GetId(pDevice));
                    auto it = steamControllerDeviceMap.find(deviceId);
                    if (it == steamControllerDeviceMap.end()) {
                        // not connected yet
                        timeval time;
                        gettimeofday(&time, NULL);
                        unsigned long millisecondsStart = (time.tv_sec * 1000) + (time.tv_usec / 1000);

                        for (;;) {
                            uint8_t res = SteamController_ReadEvent(pDevice, &event);

                            if (res == STEAMCONTROLLER_EVENT_CONNECTION &&
                                event.connection.details == STEAMCONTROLLER_CONNECTION_EVENT_DISCONNECTED) {
                                CCLOGERROR("Device %p is not connected (anymore), trying next one...\n", pDevice);
                                break;
                            }

                            if (res == STEAMCONTROLLER_EVENT_CONNECTION &&
                                event.connection.details == STEAMCONTROLLER_CONNECTION_EVENT_CONNECTED) {
                                CCLOG("DeviceId %s...\n", deviceId.c_str());
                                CCLOG("Device %p is connected, configuring...\n", pDevice);
                                SteamController_Configure(pDevice, STEAMCONTROLLER_DEFAULT_FLAGS);
                                SteamController *stub = nullptr;
                                steamControllerDeviceMap.emplace(
                                        std::make_pair(deviceId, std::make_pair(pDevice, stub)));
                                onConnected(deviceId);

                                break;
                            }

                            if (res == STEAMCONTROLLER_EVENT_BATTERY &&
                                false == SteamController_IsWirelessDongle(pDevice)) {
                                CCLOG("DeviceId %s...\n", deviceId.c_str());
                                CCLOG("Device %p is connected, configuring...\n", pDevice);
                                SteamController_Configure(pDevice, STEAMCONTROLLER_DEFAULT_FLAGS);
                                SteamController *stub = nullptr;
                                steamControllerDeviceMap.emplace(
                                        std::make_pair(deviceId, std::make_pair(pDevice, stub)));
                                onConnected(deviceId);
                                break;
                            }
                            timeval timeEnd;
                            gettimeofday(&timeEnd, NULL);
                            unsigned long millisecondsEnd = (timeEnd.tv_sec * 1000) + (timeEnd.tv_usec / 1000);
                            if (millisecondsEnd - millisecondsStart > 2000) {
                                // failed after 2 seconds
                                break;
                            }
                        }
                    }
                }
                pEnum = SteamController_NextControllerDevice(pEnum);
            }
        }

        void onDisconnect(const std::string &deviceId) {
            auto it = steamControllerDeviceMap.find(deviceId);
            if (steamControllerDeviceMap.end() != it) {
                disconnect(it->second);
                steamControllerDeviceMap.erase(deviceId);
            }
        }

        void disconnect(std::pair<SteamControllerDevice *, SteamController *> &pair) {
            SteamControllerDevice *pDevice = pair.first;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
            /*AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_IO, [](void*) {}, nullptr, [pDevice]()
        {
            SteamController_Close(pDevice);
        });*/
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
            SteamController_Close(pDevice);
#endif
            if (pair.second != nullptr) {
                pair.second->onDisconnected();
            }
        }

        void stopDiscoverySteamController() {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
            AsyncTaskPool::getInstance()->stopTasks(AsyncTaskPool::TaskType::TASK_IO);
#endif
            for (auto pair : steamControllerDeviceMap) {
                disconnect(pair.second);
            }
            steamControllerDeviceMap.clear();
        }

        void onConnected(const std::string &deviceId) {
            auto it = steamControllerDeviceMap.find(deviceId);
            if (steamControllerDeviceMap.end() != it) {
                auto controller = new cocos2d::SteamController();
                controller->deviceId = deviceId;
                it->second.second = controller;
                controller->onConnected();
            }
        }

        SteamController *getControllerByTag(int tag) {
            for (auto pair : steamControllerDeviceMap) {
                auto controller = pair.second.second;
                if (controller->getTag() == tag) {
                    return controller;
                }
            }
            return nullptr;
        }

        SteamController *getControllerByDeviceId(const std::string &deviceId) {
            auto it = steamControllerDeviceMap.find(deviceId);
            if (steamControllerDeviceMap.end() != it) {
                return it->second.second;
            }
            return nullptr;
        }

        void setup() {
            currentTime = DISCOVERY_INTERVAL + 0.001f;
        }

        const std::vector<SteamController *> &getAllController() {
            steamControllerList.clear();
            for (auto pair : steamControllerDeviceMap) {
                auto controller = pair.second.second;
                steamControllerList.emplace_back(controller);
            }
            return steamControllerList;
        }

    private:
#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        SteamControllerEvent **events;
        uint8_t *eventsRawData;
#endif
        float currentTime;
        std::unordered_map<std::string, std::pair<SteamControllerDevice *, SteamController *>> steamControllerDeviceMap;
        std::vector<SteamController *> steamControllerList;
    };

    SteamControllerImpl *SteamControllerImpl::s_SteamControllerImpl = nullptr;


    void SteamController::exit() {
        SteamControllerImpl::destroyInstance();
    }

    const std::vector<SteamController *> &SteamController::getAllController() {
        SteamControllerImpl *instance = SteamControllerImpl::getInstance();
        return instance->getAllController();
    }


    void SteamController::startDiscoverySteamController() {
        SteamControllerImpl *instance = SteamControllerImpl::getInstance();
        instance->setup();
        Director::getInstance()->getScheduler()->unscheduleUpdate(instance);
        Director::getInstance()->getScheduler()->scheduleUpdate(instance, 0, false);
    }

    void SteamController::stopDiscoverySteamController() {
        SteamControllerImpl *instance = SteamControllerImpl::getInstance();
        Director::getInstance()->getScheduler()->unscheduleUpdate(instance);
        instance->stopDiscoverySteamController();
    }


    SteamController::SteamController() : tag(TAG_UNSET),
                                         deviceId(DEVICE_UNSET),
                                         connectEvent(nullptr),
                                         updateEvent(nullptr) {
        init();
    }

    SteamController::~SteamController() {
        delete connectEvent;
        delete updateEvent;
    }


    SteamController *SteamController::getControllerByDeviceId(const std::string &deviceId) {
        return SteamControllerImpl::getInstance()->getControllerByDeviceId(deviceId);
    }

    SteamController *SteamController::getControllerByTag(int tag) {
        return SteamControllerImpl::getInstance()->getControllerByTag(tag);
    }


NS_CC_END

#endif // #if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX || CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)


