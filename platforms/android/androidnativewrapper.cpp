/*****************************************************************************
 * Copyright 2015 Haye Hinrichsen, Christoph Wick
 *
 * This file is part of Entropy Piano Tuner.
 *
 * Entropy Piano Tuner is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Entropy Piano Tuner is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Entropy Piano Tuner. If not, see http://www.gnu.org/licenses/.
 *****************************************************************************/

#include "androidnativewrapper.h"
#include "platformtools.h"
#include "core/messages/messagehandler.h"
#include "core/messages/messagemidievent.h"
#include <QDebug>


// connect java_openFile method
// on android this allows the activity to load a file
// this is necessary to support loading a file while the application is already running

void callAndroidVoidTunerApplicationFunction(const char *functionName) {
    // get the application instance
    QAndroidJniObject instance = QAndroidJniObject::callStaticObjectMethod("org/uniwue/tp3/TunerApplication", "getInstance", "()Lorg/uniwue/tp3/TunerApplication;");
    // get the path to the file to open, zero length if there is none
    instance.callMethod<void>(functionName);
}

long int callAndroidLongIntTunerApplicationFunction(const char *functionName) {
    // get the application instance
    QAndroidJniObject instance = QAndroidJniObject::callStaticObjectMethod("org/uniwue/tp3/TunerApplication", "getInstance", "()Lorg/uniwue/tp3/TunerApplication;");
    // get the path to the file to open, zero length if there is none
    return instance.callMethod<jlong>(functionName);
}

static void java_openFile(JNIEnv *env, jobject thiz, jstring string, jboolean cached) {
    (void)thiz;

    const char *file = env->GetStringUTFChars(string, 0);

    // use your string
    PlatformTools::getSingleton()->openFile(file, (bool)(cached == JNI_TRUE));

    env->ReleaseStringUTFChars(string, file);
}

static void java_sendMidiMessage(JNIEnv *env, jobject thiz, jint event, jint byte1, jint byte2, jdouble deltaTime) {
    Q_UNUSED(env);
    Q_UNUSED(thiz);
    MidiAdapter::Data data = {static_cast<MidiAdapter::MidiEvent>(event), byte1, byte2, deltaTime};
    MessageHandler::send<MessageMidiEvent>(data);
}

static JavaVM* s_javaVM = 0;

static JNINativeMethod jniNativeMethods[] = {
    {"java_openFile", "(Ljava/lang/String;Z)V", (void *)java_openFile},
    {"java_sendMidiMessage", "(IIID)V", (void *)java_sendMidiMessage}
};

// this method is called immediately after the module is load
jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    s_javaVM = vm;

    qDebug() << "Loading the jni environment";

    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        qCritical() <<"Can't get the enviroument";
        return -1;
    }

    qDebug() << "Search activity";

    // search for our class
    jclass clazz = env->FindClass("org/uniwue/tp3/TunerApplication");
    if (!clazz)
    {
        qCritical() << "Can't find TunerApplication class";
        return -1;
    }


    qDebug() << "Registering methods";

    // register our native methods
    if (env->RegisterNatives(clazz, jniNativeMethods, sizeof(jniNativeMethods) / sizeof(jniNativeMethods[0])) < 0)
    {
        qCritical() << "RegisterNatives failed";
        return -1;
    }

    qDebug() << "Java methods registered";

    return JNI_VERSION_1_6;
}
