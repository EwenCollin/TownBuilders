#include <Babylon/Plugins/NativeRTC.h>
#include <Babylon/JsRuntime.h>
#include <napi/napi_pointer.h>
#include <arcana/tracing/trace_region.h>
#include <optional>
#include <unordered_map>
#include <rtc/rtc.hpp>
#include <limits>
#include <inttypes.h>
#include <arcana/threading/task.h>
#include <arcana/threading/dispatcher.h>
#include <android/log.h>
#include <Babylon/JsRuntimeScheduler.h>

#define  LOG_TAG    "NativeLOG"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace
{

    class RTC : public Napi::ObjectWrap<RTC> {
        static constexpr auto JS_CLASS_NAME = "NativeRTC";
        static constexpr auto JS_NAVIGATOR_NAME = "navigator";
        static constexpr auto JS_RTC_NAME = "rtc";
        static constexpr auto JS_NATIVE_NAME = "native";

    public:
        static void Initialize(Napi::Env env) {
            Napi::HandleScope scope{env};

            Napi::Function func = DefineClass(
                    env,
                    JS_CLASS_NAME,
                    {
                            InstanceMethod("onPeerConnectionCreated", &RTC::AddOnPeerConnectionCreatedCallback),
                            InstanceMethod("onDataChannel", &RTC::AddOnDataChannelCallback),
                            InstanceMethod("onMessage", &RTC::AddOnMessageCallback),
                            InstanceMethod("onLocalDescription", &RTC::AddOnLocalDescriptionCallback),
                            InstanceMethod("onLocalCandidate", &RTC::AddOnLocalCandidateCallback),
                            InstanceMethod("setRemoteDescription", &RTC::SetRemoteDescription),
                            InstanceMethod("onRemoteDescriptionSet", &RTC::AddOnRemoteDescriptionSet),
                            InstanceMethod("setRemoteCandidate", &RTC::SetRemoteCandidate),
                            InstanceMethod("sendMessage", &RTC::SendMessage),
                            InstanceMethod("createDataChannel", &RTC::CreateDataChannel),
                            InstanceMethod("setConfig", &RTC::SetConfig),
                            InstanceMethod("startPeerConnection", &RTC::StartPeerConnection),
                            InstanceMethod("openWebsocket", &RTC::OpenWebsocket),
                            InstanceMethod("closeWebsocket", &RTC::CloseWebsocket),
                            InstanceMethod("onWebsocketOpen", &RTC::AddOnWebsocketOpenCallback),
                            InstanceMethod("onWebsocketMessage", &RTC::AddOnWebsocketMessageCallback),
                            InstanceMethod("onWebsocketClosed", &RTC::AddOnWebsocketClosedCallback),
                            InstanceMethod("sendWebsocketMessage", &RTC::SendWebsocketMessage),
                            InstanceMethod("startWebsocketServer", &RTC::StartWebsocketServer),
                            InstanceMethod("onWebsocketClientServer", &RTC::AddOnWebsocketClientConnectedCallback),
                            InstanceMethod("onWebsocketClientMessage", &RTC::AddOnWebsocketClientMessageCallback),
                            InstanceMethod("onWebsocketClientClosed", &RTC::AddOnWebsocketClientClosedCallback),
                            InstanceMethod("stopWebsocketServer", &RTC::StopWebsocketServer),
                            InstanceMethod("sendWebsocketServerMessage", &RTC::SendWebsocketServerMessage),
                            //InstanceAccessor("nativeXrContext", &XR::GetNativeXrContext, nullptr),
                            InstanceValue(JS_NATIVE_NAME, Napi::Value::From(env, true)),
                    });
            Napi::Object global = env.Global();
            Napi::Object navigator;
            if (global.Has(JS_NAVIGATOR_NAME)) {
                navigator = global.Get(JS_NAVIGATOR_NAME).As<Napi::Object>();
            } else {
                navigator = Napi::Object::New(env);
                global.Set(JS_NAVIGATOR_NAME, navigator);
            }

            auto rtc = func.New({});
            //RTC::Unwrap(rtc)->m_rtc = std::make_shared<RTCImpl>(env, m_runtimeScheduler);
            //XR::Unwrap(xr)->m_xr = std::move(nativeXr);
            navigator.Set(JS_RTC_NAME, rtc);
        }

        RTC(const Napi::CallbackInfo &info)
                : Napi::ObjectWrap<RTC>{info}
                ,m_jsData{Napi::Persistent(Napi::Object::New(info.Env()))}
                , m_runtimeScheduler{Babylon::JsRuntime::GetFromJavaScript(info.Env())}
        {
        }


    private:
        //JsRuntimeScheduler m_runtimeScheduler;
        //std::shared_ptr<Plugins::NativeXr::Impl> m_xr{};
        //std::shared_ptr<RTCImpl> m_rtc;
        //std::shared_ptr<Babylon::JsRuntimeScheduler> m_runtimeScheduler;

        Napi::ObjectReference m_jsData{};
        Babylon::JsRuntimeScheduler m_runtimeScheduler;

        std::vector <Napi::FunctionReference> m_callbacks{};
        std::vector <Napi::FunctionReference> m_remote_description_callbacks{};
        std::vector <Napi::FunctionReference> m_local_description_callbacks{};
        std::vector <Napi::FunctionReference> m_local_candidate_callbacks{};
        std::vector <Napi::FunctionReference> m_data_channel_callbacks{};
        std::vector <Napi::FunctionReference> m_peer_connection_created_callbacks{};
        std::vector <Napi::FunctionReference> m_message_callbacks{};
        rtc::Configuration config;
        std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> peerConnectionMap;
        std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> dataChannelMap;
        std::unordered_map<std::string, std::shared_ptr<rtc::WebSocket>> m_websockets;
        std::vector <Napi::FunctionReference> m_websocket_open_callbacks{};
        std::vector <Napi::FunctionReference> m_websocket_message_callbacks{};
        std::vector <Napi::FunctionReference> m_websocket_closed_callbacks{};
        std::shared_ptr <rtc::WebSocketServer> m_websocket_server{};
        std::map<uint32_t, std::shared_ptr<rtc::WebSocket>> m_websocket_clients{};
        std::vector <Napi::FunctionReference> m_websocket_client_connected_callbacks{};
        std::vector <Napi::FunctionReference> m_websocket_client_message_callbacks{};
        std::vector <Napi::FunctionReference> m_websocket_client_closed_callbacks{};
        uint32_t m_next_websocket_client_id;

        void AddCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();

            m_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnWebsocketOpenCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            //auto callbackPtr{std::make_shared<Napi::FunctionReference>(Napi::Persistent(listener))};
            m_websocket_open_callbacks.push_back(Napi::Persistent(listener));//std::move(callbackPtr));
        }

        void AddOnWebsocketMessageCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_websocket_message_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnWebsocketClosedCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_websocket_closed_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnDataChannelCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_data_channel_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnMessageCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_message_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnLocalDescriptionCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_local_description_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnLocalCandidateCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_local_candidate_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnWebsocketClientConnectedCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_websocket_client_connected_callbacks.push_back(Napi::Persistent(listener));
        }
        void AddOnWebsocketClientMessageCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_websocket_client_message_callbacks.push_back(Napi::Persistent(listener));
        }
        void AddOnWebsocketClientClosedCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_websocket_client_closed_callbacks.push_back(Napi::Persistent(listener));
        }
        void AddOnPeerConnectionCreatedCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_peer_connection_created_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnRemoteDescriptionSet(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_remote_description_callbacks.push_back(Napi::Persistent(listener));
        }

        void CallOnPeerConnectionCreated(const std::string id) {
            LOGD("RTC PC peer connection created callback!\n");
            for (const auto &callback : m_peer_connection_created_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, id] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("id", id);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnWebsocketOpen(const std::string url) {
            LOGD("RTCWS Websocket Open Call!\n");
            LOGD("RTCWS Websocket Open jsData ok!\n");
            for (const auto &callback : m_websocket_open_callbacks) {
                LOGD("RTCWS Websocket Open Almost there!\n");
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, url] {
                    LOGD("RTCWS Websocket Open Almost there! cb\n");
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("wsUrl", url);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnDataChannel(const std::string channel_id) {
            for (const auto &callback : m_data_channel_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, channel_id] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("channelId", channel_id);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnWebsocketMessage(const std::string url , const std::string message) {
            for (const auto &callback : m_websocket_message_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, url, message] {
                    LOGD("RTCWS Websocket on message call!\n");
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("wsUrl", url);
                    jsData.Set("message", message);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnWebsocketClosed(const std::string url) {
            for (const auto &callback : m_websocket_closed_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, url] {
                    LOGD("RTCWS Websocket on closed call!\n");
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("wsUrl", url);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnMessage(const std::string dc_index, const std::string message) {
            for (const auto &callback : m_message_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, dc_index, message] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("channelId", dc_index);
                    jsData.Set("message", message);
                    LOGD("RTC on peer message call!\n");
                    callback.Call({jsData});
                });
            }
        }

        void CallOnRemoteDescriptionSet(const std::string id) {
            for (const auto &callback : m_remote_description_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, id] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("id", id);
                    LOGD("RTC on remote description set call!\n");
                    callback.Call({jsData});
                });
            }
        }

        void CallOnLocalDescription(const std::string id, const std::string sdp_str, const std::string type) {
            for (const auto &callback : m_local_description_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, id, sdp_str, type] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("id", id);
                    jsData.Set("sdp", sdp_str);
                    jsData.Set("type", type);
                    LOGD("RTC on local description call!\n");
                    callback.Call({jsData});
                });
            }
        }

        void CallOnLocalCandidate(const std::string id, const std::string candidate_str, const std::string candidate_mid) {
            LOGD("RTC on local candidate pre-call!\n");
            for (const auto &callback : m_local_candidate_callbacks) {
                LOGD("RTC on local candidate alm-call!\n");
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, id, candidate_str, candidate_mid] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("id", id);
                    jsData.Set("candidate", candidate_str);
                    jsData.Set("mid", candidate_mid);
                    LOGD("RTC on local candidate call!\n");
                    callback.Call({jsData});
                });
            }
        }

        void SetRemoteDescription(const Napi::CallbackInfo &info) {
            LOGD("RTC PC remDesc\n");
            auto id = info[0].As<Napi::String>().Utf8Value();
            auto remote_sdp = info[1].As<Napi::String>().Utf8Value();
            auto type = info[2].As<Napi::String>().Utf8Value();
            std::shared_ptr<rtc::PeerConnection> pc;
            auto jt = peerConnectionMap.find(id);
            if (jt != peerConnectionMap.end()) {
                pc = jt->second;
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, pc, remote_sdp, type, id] {
                    LOGD("RTC PC remDesc in task\n");
                    pc->setRemoteDescription(rtc::Description(remote_sdp, type));
                    CallOnRemoteDescriptionSet(id);
                    LOGD("RTC PC remDesc after task\n");
                });
            }
        }

        void SetRemoteCandidate(const Napi::CallbackInfo &info) {
            LOGD("RTC PC remCandidate\n");
            auto id = info[0].As<Napi::String>().Utf8Value();
            auto remote_candidate = info[1].As<Napi::String>().Utf8Value();
            auto remote_mid = info[2].As<Napi::String>().Utf8Value();
            LOGD("RTC PC remCandidate params ok\n");
            std::shared_ptr<rtc::PeerConnection> pc;
            auto jt = peerConnectionMap.find(id);
            if (jt != peerConnectionMap.end()) {
                pc = jt->second;
                LOGD("RTC PC remCandidate before task\n");
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [pc, remote_candidate, remote_mid] {
                    LOGD("RTC PC remCandidate in task\n");
                    pc->addRemoteCandidate(rtc::Candidate(remote_candidate, remote_mid));
                    LOGD("RTC PC remCandidate after add cand\n");
                });
            }
        }


        void CreateDataChannel(const Napi::CallbackInfo &info) {
            auto id = info[0].As<Napi::String>().Utf8Value();
            auto channel_label = info[1].As<Napi::String>().Utf8Value();

            auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, id, channel_label] {
                auto pc = createPeerConnection(id);
                auto dc = pc->createDataChannel(channel_label);
                dataChannelMap.emplace(id, dc);
                dc->onOpen([this, id]() {
                    CallOnDataChannel(id);
                });
                dc->onMessage([this, id](std::variant<rtc::binary, rtc::string> message) {
                    if (std::holds_alternative<rtc::string>(message)) {
                        CallOnMessage(id, std::get<rtc::string>(message));
                    }
                });
                CallOnPeerConnectionCreated(id);
            });
        }

        void SendMessage(const Napi::CallbackInfo &info) {
            auto id = info[0].As<Napi::String>().Utf8Value();
            auto message = info[1].As<Napi::String>().Utf8Value();
            std::shared_ptr<rtc::DataChannel> dc;
            auto jt = dataChannelMap.find(id);
            if (jt != dataChannelMap.end()) {
                dc = jt->second;
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [dc, message] {
                    dc->send(message);
                });
            }
        }

        std::shared_ptr<rtc::PeerConnection> createPeerConnection(const std::string id) {
            auto pc = std::make_shared<rtc::PeerConnection>(config);
            peerConnectionMap.emplace(id, pc);

            pc->onLocalDescription([this, id](rtc::Description sdp) {
                // Send the SDP to the remote peer
                LOGD("RTC on local description re-call!\n");
                CallOnLocalDescription(id, std::string(sdp), sdp.typeString());
            });

            pc->onLocalCandidate([this, id](rtc::Candidate candidate) {
                // Send the candidate to the remote peer
                LOGD("RTC on local candidate re-call!\n");
                CallOnLocalCandidate(id, std::string(candidate), candidate.mid());
            });
            pc->onDataChannel([this, id](std::shared_ptr <rtc::DataChannel> incoming) {
                LOGD("RTC on local channel setup!\n");
                dataChannelMap.emplace(id, std::move(incoming));
                std::shared_ptr<rtc::DataChannel> dc;
                auto jt = dataChannelMap.find(id);
                if (jt != dataChannelMap.end()) {
                    dc = jt->second;
                    dc->onOpen([this, id]() {
                        LOGD("RTC local channel open!\n");
                        CallOnDataChannel(id);
                    });
                    //onClosed
                    //incoming->send("Hello world!");
                    LOGD("RTC set on message channel!\n");
                    dc->onMessage([this, id](std::variant <rtc::binary, rtc::string> message) {
                        if (std::holds_alternative<rtc::string>(message)) {
                            LOGD("On RTC Message PeerConn call\n");
                            CallOnMessage(id, std::get<rtc::string>(message));
                        }
                    });
                }
                LOGD("RTC local channel open emplace!\n");

            });
            return pc;
        }

        void SetConfig(const Napi::CallbackInfo &info) {
            auto turn_server = info[0].As<Napi::String>().Utf8Value();
            config.iceServers.emplace_back(turn_server);

        }

        void StartPeerConnection(const Napi::CallbackInfo &info) {
            auto id = info[0].As<Napi::String>().Utf8Value();
            LOGD("RTC PC start\n");
            auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, id] {
                auto pc = createPeerConnection(id);
                CallOnPeerConnectionCreated(id);
                LOGD("RTC after peer connection created in call!\n");
            });
        }

        void SendWebsocketMessage(const Napi::CallbackInfo &info) {
            auto ws_url = info[0].As<Napi::String>().Utf8Value();
            auto message = info[1].As<Napi::String>().Utf8Value();
            std::shared_ptr<rtc::WebSocket> ws;
            auto jt = m_websockets.find(ws_url);
            if (jt != m_websockets.end()) {
                ws = jt->second;
                LOGD("RTCWS before ws->send!\n");
                //auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [ws, message] {
                    LOGD("RTCWS before ws->send in async task!\n");
                    ws->send(message);
                //});
            }
        }

        void CloseWebsocket(const Napi::CallbackInfo &info) {
            LOGD("RTCWS called close websocket\n");
            auto ws_url = info[0].As<Napi::String>().Utf8Value();
            std::shared_ptr<rtc::WebSocket> ws;
            auto jt = m_websockets.find(ws_url);
            if (jt != m_websockets.end()) {
                ws = jt->second;
                LOGD("RTCWS closing ws->close async task!\n");
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [ws] {
                    LOGD("RTCWS closing ws->close!\n");
                    ws->close();
                });
            }
        }

        void OpenWebsocket(const Napi::CallbackInfo &info) {
            LOGD("RTCWS before websocket\n");
            auto url = info[0].As<Napi::String>().Utf8Value();
            auto ws = std::make_shared<rtc::WebSocket>();
            //arcana::background_dispatcher<32> scheduler;
            auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, url, ws] {
                LOGD("RTCWS Opening websocket\n");
                ws->onOpen([this, url]() {
                    LOGD("RTCWS Websocket Open!\n");
                    //auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, url] {

                        LOGD("RTCWS Websocket Open CB!\n");
                        CallOnWebsocketOpen(url);
                    //});
                });
                ws->onMessage([this, url](std::variant<rtc::binary, rtc::string> message) {
                    if (std::holds_alternative<rtc::string>(message)) {
                        LOGD("RTCWS Websocket Message before task!\n");
                        //auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, url, message] {
                            LOGD("RTCWS Websocket Message CB!\n");
                            CallOnWebsocketMessage(url, std::get<rtc::string>(message));
                        //});
                    }
                });
                ws->onError([this, url](std::string s) {
                    (void)s;
                    //LOGD("RTCWS Websocket Error! - %s\n", s.c_str());
                    //auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, url] {
                        LOGD("RTCWS Websocket Error CB!\n");
                        CallOnWebsocketClosed(url);
                    //});
                });
                ws->onClosed([this, url]() {
                    LOGD("RTCWS Websocket Closed CB!\n");
                    //auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, url] {
                        CallOnWebsocketClosed(url);
                    //});
                });
                ws->open(url);
                m_websockets.emplace(url, std::move(ws));//.push_back(std::move(ws));
            });
        }

        std::string getOrNullRemoteAddressWebsocket(rtc::WebSocket &wsClient) {
            auto remoteAddress = wsClient.remoteAddress();
            std::string retAddress = static_cast<std::string>(remoteAddress.value_or("null"));
            return retAddress;
        }

        void CallOnWebsocketClientConnected(uint32_t clientId, std::string remoteAddress) {
            Napi::Object jsData = m_jsData.Value();
            jsData.Set("clientId", clientId);
            jsData.Set("remoteAddress", remoteAddress);
            for (const auto &callback : m_websocket_client_connected_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [&callback, jsData] {
                    callback.Call({jsData});
                });
            }
        }
        void CallOnWebsocketClientMessage(uint32_t clientId, std::string remoteAddress, std::string message) {
            Napi::Object jsData = m_jsData.Value();
            jsData.Set("clientId", clientId);
            jsData.Set("remoteAddress", remoteAddress);
            jsData.Set("message", message);
            for (const auto &callback : m_websocket_client_message_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [&callback, jsData] {
                    callback.Call({jsData});
                });
            }
        }
        void CallOnWebsocketClientClosed(uint32_t clientId, std::string remoteAddress) {
            Napi::Object jsData = m_jsData.Value();
            jsData.Set("clientId", clientId);
            jsData.Set("remoteAddress", remoteAddress);
            for (const auto &callback : m_websocket_client_closed_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [&callback, jsData] {
                    callback.Call({jsData});
                });
            }
        }


        void SendWebsocketServerMessage(const Napi::CallbackInfo &info) {
            auto clientId = info[0].As<Napi::Number>().Uint32Value();
            auto message = info[1].As<Napi::String>().Utf8Value();
            std::shared_ptr<rtc::WebSocket> ws;
            auto jt = m_websocket_clients.find(clientId);
            if (jt != m_websocket_clients.end()) {
                ws = jt->second;
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [ws, message] {
                    ws->send(message);
                });
            }
        }

        void StopWebsocketServer(const Napi::CallbackInfo &info) {
            (void)info;
            auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this] {
                if (m_websocket_server != nullptr) m_websocket_server->stop();
            });
        }

        void StartWebsocketServer(const Napi::CallbackInfo &info) {
            auto port = info[0].As<Napi::Number>().Uint32Value();
            rtc::WebSocketServer::Configuration serverConfig = {};
            serverConfig.port = static_cast<u_int16_t>(port);
            serverConfig.enableTls = true;
            auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, serverConfig] {
                auto wsServer = std::make_shared<rtc::WebSocketServer>(serverConfig);
                wsServer->onClient([this](std::shared_ptr<rtc::WebSocket> wsClient) {
                    auto clientId = m_next_websocket_client_id++;
                    LOGD("RTCSERVER on new Client connected!\n");
                    wsClient->onMessage([this, clientId, wsClient](std::variant<rtc::binary, rtc::string> message) {
                        if (std::holds_alternative<rtc::string>(message)) {
                            auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, clientId, wsClient, message] {
                                CallOnWebsocketClientMessage(clientId, getOrNullRemoteAddressWebsocket(*wsClient), std::get<rtc::string>(message));
                            });
                        }
                    });
                    wsClient->onError([this, clientId, wsClient](std::string s) {
                        LOGD("RTCWS Websocket Error! - %s\n", s.c_str());
                        auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, clientId, wsClient] {
                            LOGD("RTCSERVER Client Websocket Error CB!\n");
                            CallOnWebsocketClientClosed(clientId, getOrNullRemoteAddressWebsocket(*wsClient));
                            m_websocket_clients.erase(clientId);
                        });
                    });
                    wsClient->onClosed([this, clientId, wsClient]() {
                        LOGD("RTCSERVER Client Websocket Closed CB!\n");
                        auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, clientId, wsClient] {
                            CallOnWebsocketClientClosed(clientId, getOrNullRemoteAddressWebsocket(*wsClient));
                            m_websocket_clients.erase(clientId);
                            //std::remove(m_websocket_clients.begin(), m_websocket_clients.end(), wsClient);
                        });
                    });
                    m_websocket_clients.emplace(clientId, std::move(wsClient));
                    auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(),
                          [this, clientId, wsClient] {
                              LOGD("RTCSERVER onClient CB!\n");
                              CallOnWebsocketClientConnected(clientId, getOrNullRemoteAddressWebsocket(*wsClient));
                          });
                });
                m_websocket_server = std::move(wsServer);
                m_websocket_clients.clear();
                m_next_websocket_client_id = 0;
            });
        }

        /*
        void AddCallback(const Napi::CallbackInfo &info) {
            m_rtc->AddCallback(info);
        }

        void AddOnWebsocketOpenCallback(const Napi::CallbackInfo &info) {
            m_rtc->AddOnWebsocketOpenCallback(info);
        }

        void AddOnWebsocketMessageCallback(const Napi::CallbackInfo &info) {
            m_rtc->AddOnWebsocketMessageCallback(info);
        }

        void AddOnWebsocketClosedCallback(const Napi::CallbackInfo &info) {
            m_rtc->AddOnWebsocketClosedCallback(info);
        }

        void AddOnDataChannelCallback(const Napi::CallbackInfo &info) {
            m_rtc->AddOnDataChannelCallback(info);
        }

        void AddOnMessageCallback(const Napi::CallbackInfo &info) {
            m_rtc->AddOnMessageCallback(info);
        }

        void AddOnLocalDescriptionCallback(const Napi::CallbackInfo &info) {
            m_rtc->AddOnLocalDescriptionCallback(info);
        }

        void AddOnLocalCandidateCallback(const Napi::CallbackInfo &info) {
            m_rtc->AddOnLocalCandidateCallback(info);
        }


        void SetRemoteDescription(const Napi::CallbackInfo &info) {
            m_rtc->SetRemoteDescription(info);
        }

        void SetRemoteCandidate(const Napi::CallbackInfo &info) {
            m_rtc->SetRemoteCandidate(info);
        }


        void CreateDataChannel(const Napi::CallbackInfo &info) {
            m_rtc->CreateDataChannel(info);
        }

        void SendMessage(const Napi::CallbackInfo &info) {
            m_rtc->SendMessage(info);
        }

        void SetConfig(const Napi::CallbackInfo &info) {
            m_rtc->SetConfig(info);

        }

        void StartPeerConnection(const Napi::CallbackInfo &info) {
            m_rtc->StartPeerConnection(info);
        }

        void SendWebsocketMessage(const Napi::CallbackInfo &info) {
            m_rtc->SendWebsocketMessage(info);
        }

        void OpenWebsocket(const Napi::CallbackInfo &info) {
            m_rtc->OpenWebsocket(info);
        }
        */
    };
}

namespace Babylon::Plugins::NativeRTC
{
    void Initialize(Napi::Env env)
    {
        RTC::Initialize(env);
        /*
        auto nativeObject{JsRuntime::NativeObject::GetFromJavaScript(env)};
        nativeObject.Set("startPerformanceCounter", Napi::Function::New(env, StartPerformanceCounter, "startPerformanceCounter"));
        nativeObject.Set("endPerformanceCounter", Napi::Function::New(env, EndPerformanceCounter, "endPerformanceCounter"));
        nativeObject.Set("enablePerformanceLogging", Napi::Function::New(env, EnablePerformanceTracing, "enablePerformanceLogging"));
        nativeObject.Set("disablePerformanceLogging", Napi::Function::New(env, DisablePerformanceTracing, "disablePerformanceLogging"));*/
    }
}
