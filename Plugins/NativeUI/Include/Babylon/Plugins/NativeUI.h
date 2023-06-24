#pragma once

#include <napi/env.h>
#include <jni.h>

namespace Babylon::Plugins::NativeUI
{
    void Initialize(Napi::Env env);//, JNIEnv* jniEnv);
    //void SetActivity(jobject activity);
}
