#include <Babylon/Plugins/NativeUI.h>
#include <Babylon/JsRuntime.h>
#include <napi/napi_pointer.h>
#include <arcana/tracing/trace_region.h>
#include <optional>
#include <unordered_map>
#include <limits>
#include <inttypes.h>
#include <arcana/threading/task.h>
#include <arcana/threading/dispatcher.h>
#include <android/log.h>
#include <Babylon/JsRuntimeScheduler.h>
#include <jni.h>

#include <AndroidExtensions/Globals.h>

#define  LOG_TAG    "NativeLOG"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace
{

    class UI : public Napi::ObjectWrap<UI> {
        static constexpr auto JS_CLASS_NAME = "NativeUI";
        static constexpr auto JS_NAVIGATOR_NAME = "navigator";
        static constexpr auto JS_UI_NAME = "ui";
        static constexpr auto JS_NATIVE_NAME = "native";

    public:

        /*
        static void SetActivity(jobject activity) {
            LOGD("Call set ACTUIs\n");
            UI::s_activity = activity;
        }
         */

        static void Initialize(Napi::Env env) {
            Napi::HandleScope scope{env};

            //UI::s_jniEnv = jniEnv;

            Napi::Function func = DefineClass(
                    env,
                    JS_CLASS_NAME,
                    {
                            InstanceMethod("onUIAvailable", &UI::AddOnUIAvailableCallback),
                            InstanceMethod("onLogin", &UI::AddOnLoginCallback),
                            InstanceMethod("onRegister", &UI::AddOnRegisterCallback),
                            InstanceMethod("showLogin", &UI::ShowLogin),
                            InstanceMethod("setLoginError", &UI::SetLoginError),
                            InstanceMethod("setRegisterError", &UI::SetRegisterError),
                            InstanceMethod("clearLoginError", &UI::ClearLoginError),
                            InstanceMethod("clearRegisterError", &UI::ClearRegisterError),
                            InstanceMethod("hideLogin", &UI::HideLogin),
                            InstanceMethod("hideRegister", &UI::HideRegister),
                            InstanceMethod("hideHomeContainer", &UI::HideHomeContainer),
                            InstanceMethod("onLoginError", &UI::AddOnLoginErrorCallback),
                            InstanceMethod("onRegisterError", &UI::AddOnRegisterErrorCallback),
                            InstanceMethod("callOnLoginError", &UI::CallOnLoginError),
                            InstanceMethod("callOnLogin", &UI::CallOnLogin),
                            InstanceMethod("callOnRegister", &UI::CallOnRegister),
                            InstanceMethod("callOnUIAvailable", &UI::CallOnUIAvailable),
                            InstanceMethod("callOnTabSelectTiles", &UI::CallOnTabSelectTiles),
                            InstanceMethod("callOnTabSelectMap", &UI::CallOnTabSelectMap),
                            InstanceMethod("callOnTabSelectInventory", &UI::CallOnTabSelectInventory),
                            InstanceMethod("callOnTabSelectCamera", &UI::CallOnTabSelectCamera),
                            InstanceMethod("onTabSelectTiles", &UI::AddOnTabSelectTilesCallback),
                            InstanceMethod("onTabSelectMap", &UI::AddOnTabSelectMapCallback),
                            InstanceMethod("onTabSelectInventory", &UI::AddOnTabSelectInventoryCallback),
                            InstanceMethod("onTabSelectCamera", &UI::AddOnTabSelectCameraCallback),
                            InstanceMethod("showTabView", &UI::ShowTabView),
                            InstanceMethod("showTextOverlay", &UI::ShowTextOverlay),
                            InstanceMethod("setTextOverlayTransform", &UI::SetTextOverlayTransform),
                            InstanceMethod("setTextOverlayText", &UI::SetTextOverlayText),
                            InstanceMethod("hideTextOverlay", &UI::HideTextOverlay),
                            InstanceMethod("addMapCard", &UI::AddMapCard),
                            InstanceMethod("removeMapCard", &UI::RemoveMapCard),
                            InstanceMethod("setMapCardTranslation", &UI::SetMapCardTranslation),
                            InstanceMethod("callOnMapCardSelected", &UI::CallOnMapCardSelected),
                            InstanceMethod("onMapCardSelected", &UI::AddOnMapCardSelectedCallback),
                            InstanceMethod("showHotbar", &UI::ShowHotbar),
                            InstanceMethod("hideHotbar", &UI::HideHotbar),
                            InstanceMethod("setHotbarItems", &UI::SetHotbarItems),
                            InstanceMethod("onHotbarItemClick", &UI::AddOnHotbarItemClickCallback),
                            InstanceMethod("callOnHotbarItemClick", &UI::CallOnHotbarItemClick),
                            InstanceMethod("showClaimTile", &UI::ShowClaimTile),
                            InstanceMethod("hideClaimTile", &UI::HideClaimTile),
                            InstanceMethod("callOnClaimCurrentTile", &UI::CallOnClaimCurrentTile),
                            InstanceMethod("onClaimCurrentTile", &UI::AddOnClaimCurrentTileCallback),
                            InstanceMethod("callOnCancelClaimCurrentTile", &UI::CallOnCancelClaimCurrentTile),
                            InstanceMethod("onCancelClaimCurrentTile", &UI::AddOnCancelClaimCurrentTileCallback),
                            InstanceMethod("callOnCreateNewTile", &UI::CallOnCreateNewTile),
                            InstanceMethod("onCreateNewTile", &UI::AddOnCreateNewTileCallback),


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

            auto ui = func.New({});
            //RTC::Unwrap(rtc)->m_rtc = std::make_shared<RTCImpl>(env, m_runtimeScheduler);
            //XR::Unwrap(xr)->m_xr = std::move(nativeXr);
            navigator.Set(JS_UI_NAME, ui);
        }

        UI(const Napi::CallbackInfo &info)
                : Napi::ObjectWrap<UI>{info}
                ,m_jsData{Napi::Persistent(Napi::Object::New(info.Env()))}
                , m_runtimeScheduler{Babylon::JsRuntime::GetFromJavaScript(info.Env())}
        {
        }


    private:

        //static inline JNIEnv* s_jniEnv;
        //static inline jobject s_activity;

        Napi::ObjectReference m_jsData{};
        Babylon::JsRuntimeScheduler m_runtimeScheduler;

        std::vector <Napi::FunctionReference> m_ui_available_callbacks{};
        std::vector <Napi::FunctionReference> m_login_callbacks{};
        std::vector <Napi::FunctionReference> m_login_error_callbacks{};
        std::vector <Napi::FunctionReference> m_register_callbacks{};
        std::vector <Napi::FunctionReference> m_register_error_callbacks{};
        std::vector <Napi::FunctionReference> m_tabselect_tiles_callbacks{};
        std::vector <Napi::FunctionReference> m_tabselect_map_callbacks{};
        std::vector <Napi::FunctionReference> m_tabselect_inventory_callbacks{};
        std::vector <Napi::FunctionReference> m_tabselect_camera_callbacks{};
        std::vector <Napi::FunctionReference> m_map_card_selected_callbacks{};
        std::vector <Napi::FunctionReference> m_hotbar_item_click_callbacks{};
        std::vector <Napi::FunctionReference> m_claim_current_tile_callbacks{};
        std::vector <Napi::FunctionReference> m_cancel_claim_current_tile_callbacks{};
        std::vector <Napi::FunctionReference> m_create_new_tile_callbacks{};

        bool m_is_ui_available = false;


        void AddOnUIAvailableCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_ui_available_callbacks.push_back(Napi::Persistent(listener));
        }

        void CallOnUIAvailable(const Napi::CallbackInfo &info) {
            LOGD("NatUI CallOnUIAvailable!\n");
            m_is_ui_available = true;
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_ui_available_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void AddOnClaimCurrentTileCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_claim_current_tile_callbacks.push_back(Napi::Persistent(listener));
        }

        void CallOnClaimCurrentTile(const Napi::CallbackInfo &info) {
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_claim_current_tile_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void AddOnCancelClaimCurrentTileCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_cancel_claim_current_tile_callbacks.push_back(Napi::Persistent(listener));
        }
        void CallOnCancelClaimCurrentTile(const Napi::CallbackInfo &info) {
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_cancel_claim_current_tile_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }
        void AddOnCreateNewTileCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_create_new_tile_callbacks.push_back(Napi::Persistent(listener));
        }
        void CallOnCreateNewTile(const Napi::CallbackInfo &info) {
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_create_new_tile_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void AddOnHotbarItemClickCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_hotbar_item_click_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnTabSelectTilesCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_tabselect_tiles_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnMapCardSelectedCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_map_card_selected_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnTabSelectMapCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_tabselect_map_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnTabSelectInventoryCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_tabselect_inventory_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnTabSelectCameraCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_tabselect_camera_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnLoginCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_login_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnRegisterCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_register_callbacks.push_back(Napi::Persistent(listener));
        }

        void CallOnHotbarItemClick(const Napi::CallbackInfo &info) {
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_hotbar_item_click_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnTabSelectTiles(const Napi::CallbackInfo &info) {
            LOGD("NatUIL CallOnTabSelectTiles!\n");
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_tabselect_tiles_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnMapCardSelected(const Napi::CallbackInfo &info) {
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_map_card_selected_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }

        }

        void CallOnTabSelectMap(const Napi::CallbackInfo &info) {
            LOGD("NatUIL CallOnTabSelectMap!\n");
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_tabselect_map_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }
        void CallOnTabSelectInventory(const Napi::CallbackInfo &info) {
            LOGD("NatUIL CallOnTabSelectInventory!\n");
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_tabselect_inventory_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }
        void CallOnTabSelectCamera(const Napi::CallbackInfo &info) {
            LOGD("NatUIL CallOnTabSelectCamera!\n");
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_tabselect_camera_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnLogin(const Napi::CallbackInfo &info) {
            LOGD("NatUIL CallOnLogin!\n");
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_login_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnRegister(const Napi::CallbackInfo &info) {
            LOGD("NatUIL CallOnRegister!\n");
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_register_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void AddOnLoginErrorCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_login_error_callbacks.push_back(Napi::Persistent(listener));
        }

        void AddOnRegisterErrorCallback(const Napi::CallbackInfo &info) {
            auto listener = info[0].As<Napi::Function>();
            m_register_error_callbacks.push_back(Napi::Persistent(listener));
        }


        void CallOnLoginError(const Napi::CallbackInfo &info) {
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_login_error_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void CallOnRegisterError(const Napi::CallbackInfo &info) {
            auto payload = info[0].As<Napi::String>().Utf8Value();
            for (const auto &callback : m_register_error_callbacks) {
                auto task = arcana::make_task(m_runtimeScheduler, arcana::cancellation::none(), [this, &callback, payload] {
                    Napi::Object jsData = m_jsData.Value();
                    jsData.Set("payload", payload);
                    callback.Call({jsData});
                });
            }
        }

        void CallVoidJNIMethodString2(const std::string methodName, const std::string strParam, const std::string strParam2) {
            if (!m_is_ui_available) return;
            auto s_jniEnv = android::global::GetEnvForCurrentThread();
            auto s_activity = android::global::GetCurrentActivity();
            jstring jStr = s_jniEnv->NewStringUTF(strParam.c_str());
            jstring jStr2 = s_jniEnv->NewStringUTF(strParam2.c_str());
            jclass cls_activity = s_jniEnv->GetObjectClass(s_activity);
            jmethodID mid_callback = s_jniEnv->GetMethodID(cls_activity, methodName.c_str(), "(Ljava/lang/String;Ljava/lang/String;)V");
            s_jniEnv->CallVoidMethod(s_activity, mid_callback, jStr, jStr2);
        }
        void CallVoidJNIMethodStringFloat2(const std::string methodName, const std::string strParam, float f1, float f2) {
            if (!m_is_ui_available) return;
            auto s_jniEnv = android::global::GetEnvForCurrentThread();
            auto s_activity = android::global::GetCurrentActivity();
            jstring jStr = s_jniEnv->NewStringUTF(strParam.c_str());
            jclass cls_activity = s_jniEnv->GetObjectClass(s_activity);
            jmethodID mid_callback = s_jniEnv->GetMethodID(cls_activity, methodName.c_str(), "(Ljava/lang/String;FF)V");
            s_jniEnv->CallVoidMethod(s_activity, mid_callback, jStr, f1, f2);
        }

        void CallVoidJNIMethod(const std::string methodName) {
            if (!m_is_ui_available) return;
            auto s_jniEnv = android::global::GetEnvForCurrentThread();
            auto s_activity = android::global::GetCurrentActivity();
            jclass cls_activity = s_jniEnv->GetObjectClass(s_activity);
            jmethodID mid_callback = s_jniEnv->GetMethodID(cls_activity, methodName.c_str(), "()V");
            s_jniEnv->CallVoidMethod(s_activity, mid_callback);
        }

        void CallVoidJNIMethodString(const std::string methodName, const std::string strParam) {
            if (!m_is_ui_available) return;
            auto s_jniEnv = android::global::GetEnvForCurrentThread();
            auto s_activity = android::global::GetCurrentActivity();
            jstring jStr = s_jniEnv->NewStringUTF(strParam.c_str());
            jclass cls_activity = s_jniEnv->GetObjectClass(s_activity);
            jmethodID mid_callback = s_jniEnv->GetMethodID(cls_activity, methodName.c_str(), "(Ljava/lang/String;)V");
            s_jniEnv->CallVoidMethod(s_activity, mid_callback, jStr);
        }

        void CallVoidJNIMethodFloat3(const std::string methodName, const float f1, const float f2, const float f3) {
            if (!m_is_ui_available) return;
            auto s_jniEnv = android::global::GetEnvForCurrentThread();
            auto s_activity = android::global::GetCurrentActivity();
            jclass cls_activity = s_jniEnv->GetObjectClass(s_activity);
            jmethodID mid_callback = s_jniEnv->GetMethodID(cls_activity, methodName.c_str(), "(FFF)V");
            s_jniEnv->CallVoidMethod(s_activity, mid_callback, f1, f2, f3);
        }

        void ShowClaimTile(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("showClaimTile");
        }
        void HideClaimTile(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("hideClaimTile");
        }

        void SetTextOverlayTransform(const Napi::CallbackInfo &info) {
            auto x = info[0].As<Napi::Number>().FloatValue();
            auto y = info[1].As<Napi::Number>().FloatValue();
            auto scale = info[2].As<Napi::Number>().FloatValue();
            CallVoidJNIMethodFloat3("setTextOverlayTransform", x, y, scale);
        }

        void ShowTextOverlay(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("showTextOverlay");
        }

        void HideTextOverlay(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("hideTextOverlay");
        }

        void SetTextOverlayText(const Napi::CallbackInfo &info) {
            auto textStr = info[0].As<Napi::String>().Utf8Value();
            CallVoidJNIMethodString("setTextOverlayText", textStr);
        }

        void ShowHotbar(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("showHotbar");
        }
        void HideHotbar(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("hideHotbar");
        }

        void SetHotbarItems(const Napi::CallbackInfo &info) {
            auto hotbarItemsStr = info[0].As<Napi::String>().Utf8Value();
            CallVoidJNIMethodString("setHotbarItems", hotbarItemsStr);
        }

        void ShowLogin(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("showLogin");
        }
        void ShowTabView(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("showTabView");
        }

        void HideLogin(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("hideLogin");
        }

        void HideRegister(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("hideRegister");
        }
        void HideHomeContainer(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("hideHomeContainer");
        }

        void ClearLoginError(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("clearLoginError");
        }

        void ClearRegisterError(const Napi::CallbackInfo &info) {
            (void) info;
            CallVoidJNIMethod("clearRegisterError");
        }

        void SetLoginError(const Napi::CallbackInfo &info) {
            auto errorStr = info[0].As<Napi::String>().Utf8Value();
            CallVoidJNIMethodString("setLoginError", errorStr);
        }

        void SetRegisterError(const Napi::CallbackInfo &info) {
            auto errorStr = info[0].As<Napi::String>().Utf8Value();
            CallVoidJNIMethodString("setRegisterError", errorStr);
        }

        void AddMapCard(const Napi::CallbackInfo &info) {
            auto idStr = info[0].As<Napi::String>().Utf8Value();
            auto nameStr = info[1].As<Napi::String>().Utf8Value();
            CallVoidJNIMethodString2("addMapCard", idStr, nameStr);
        }

        void RemoveMapCard(const Napi::CallbackInfo &info) {
            auto idStr = info[0].As<Napi::String>().Utf8Value();
            CallVoidJNIMethodString("removeMapCard", idStr);
        }

        void SetMapCardTranslation(const Napi::CallbackInfo &info) {
            auto idStr = info[0].As<Napi::String>().Utf8Value();
            auto x = info[1].As<Napi::Number>().FloatValue();
            auto y = info[2].As<Napi::Number>().FloatValue();
            CallVoidJNIMethodStringFloat2("setMapCardTranslation", idStr, x, y);
        }

    };
}

namespace Babylon::Plugins::NativeUI
{
    /*
    void SetActivity(jobject activity) {
        UI::SetActivity(activity);
    }
     */

    void Initialize(Napi::Env env)
    {
        UI::Initialize(env);

    }
}
