/*
 Copyright 2014 Smartsheet.com, Inc.
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include "stdpch.h"
#include "smjni/java_runtime.h"

using namespace smjni;

java_runtime * java_runtime::s_instance = nullptr;

java_runtime::java_runtime(JNIEnv * jenv):
    m_object(jenv),
    m_throwable(jenv)
{}

void java_runtime::init(JNIEnv * env)
{
    s_instance = new java_runtime(env);
}

void java_runtime::term()
{
    delete s_instance;
}


