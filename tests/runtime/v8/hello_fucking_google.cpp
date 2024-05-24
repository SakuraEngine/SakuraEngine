// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-exception.h"
#include "v8-external.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-microtask-queue.h"
#include "v8-primitive.h"
#include "v8-script.h"
#include "v8-template.h"
#include "v8-function.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include "SkrV8/v8_isolate.hpp"
#include "SkrV8/v8_context.hpp"
#include "SkrCore/exec_static.hpp"
#include "SkrRTTR/export/record_builder.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrCore/log.hpp"

// user data
struct MyData {
    std::string content = "fuck u google";
    uint32_t    times   = 114514;

    ~MyData()
    {
        printf("i am die {content: %s, times: %d}\n", content.c_str(), times);
    }
};
static std::unordered_map<void*, v8::UniquePersistent<v8::Value>> my_data_instances;
static v8::UniquePersistent<v8::Promise::Resolver>                global_resolver;

// old test function
int old_main(int argc, char* argv[])
{
    // Initialize V8.
    char Flags[] = "--expose-gc";
    v8::V8::SetFlagsFromString(Flags, sizeof(Flags));
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    // Create a new Isolate and make it the current one.
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate                 = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope(isolate);
        // Create a new context.
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        // setup global
        auto global = context->Global();
        {

            // bind user data template
            static auto my_data_ctor_template = v8::FunctionTemplate::New(
                isolate, +[](const v8::FunctionCallbackInfo<v8::Value>& info) {
                    v8::Isolate*           Isolate = info.GetIsolate();
                    v8::Isolate::Scope     IsolateScope(Isolate);
                    v8::HandleScope        HandleScope(Isolate);
                    v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
                    v8::Context::Scope     ContextScope(Context);

                    if (info.IsConstructCall())
                    {
                        auto                  self          = info.This();
                        [[maybe_unused]] auto holder        = info.Holder();
                        auto                  param_content = info[0];
                        auto                  param_times   = info[1];
                        auto                  new_data      = new MyData();
                        if (!param_content->IsUndefined())
                        {
                            new_data->content = *v8::String::Utf8Value(info.GetIsolate(), param_content);
                        }
                        if (!param_times->IsUndefined())
                        {
                            new_data->times = param_times->Uint32Value(Context).ToChecked();
                        }
                        self->SetInternalField(0, v8::External::New(info.GetIsolate(), new_data));
                        my_data_instances.try_emplace(new_data);
                        auto& ref = my_data_instances[new_data];
                        ref.Reset(Isolate, self);
                        ref.SetWeak<MyData>(
                            new_data,
                            +[](const v8::WeakCallbackInfo<MyData>& data) {
                                auto ptr = data.GetParameter();
                                printf("==========> call destruct <==========");
                                delete ptr;
                                my_data_instances.erase(my_data_instances.find(ptr));
                            },
                            v8::WeakCallbackType::kInternalFields);
                    }
                    else
                    {
                        Isolate->ThrowException(v8::Exception::Error(
                            v8::String::NewFromUtf8Literal(Isolate, "only call as Construct is supported!")));
                    }
                });
            v8::Local<v8::ObjectTemplate> my_data_template = my_data_ctor_template->InstanceTemplate();
            my_data_template->SetInternalFieldCount(1);
            my_data_template->SetAccessor(
                v8::String::NewFromUtf8Literal(isolate, "content"),
                +[](v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
                    v8::Local<v8::Object>   self = info.Holder();
                    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
                    MyData*                 ptr  = reinterpret_cast<MyData*>(wrap->Value());
                    info.GetReturnValue().Set(
                        v8::String::NewFromUtf8(info.GetIsolate(), ptr->content.c_str()).ToLocalChecked());
                },
                +[](v8::Local<v8::String> property, v8::Local<v8::Value> value,
                    const v8::PropertyCallbackInfo<void>& info) {
                    v8::Local<v8::Object>   self = info.Holder();
                    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
                    MyData*                 ptr  = reinterpret_cast<MyData*>(wrap->Value());
                    ptr->content                 = *v8::String::Utf8Value(info.GetIsolate(), value);
                });
            my_data_template->SetAccessor(
                v8::String::NewFromUtf8Literal(isolate, "times"),
                +[](v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
                    v8::Local<v8::Object>   self = info.Holder();
                    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
                    MyData*                 ptr  = reinterpret_cast<MyData*>(wrap->Value());
                    info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), ptr->times));
                },
                +[](v8::Local<v8::String> property, v8::Local<v8::Value> value,
                    const v8::PropertyCallbackInfo<void>& info) {
                    v8::Local<v8::Object>   self = info.Holder();
                    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
                    MyData*                 ptr  = reinterpret_cast<MyData*>(wrap->Value());
                    ptr->times                   = value->Uint32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
                });

            global->Set(context, v8::String::NewFromUtf8Literal(isolate, "MyData"),
                        my_data_ctor_template->GetFunction(context).ToLocalChecked())
                .Check();
            global->Set(context, v8::String::NewFromUtf8Literal(isolate, "my_print"),
                        v8::FunctionTemplate::New(
                            isolate, +[](const v8::FunctionCallbackInfo<v8::Value>& info) {
                                v8::Isolate*          isolate = info.GetIsolate();
                                v8::HandleScope       handle_scope(isolate);
                                v8::String::Utf8Value utf8(isolate, info[0]);
                                printf("i say: %s\n", *utf8);
                            })
                            ->GetFunction(context)
                            .ToLocalChecked())
                .Check();
            global->Set(context, v8::String::NewFromUtf8Literal(isolate, "wait_call"),
                        v8::FunctionTemplate::New(
                            isolate, +[](const v8::FunctionCallbackInfo<v8::Value>& info) {
                                v8::Isolate*    isolate = info.GetIsolate();
                                v8::HandleScope handle_scope(isolate);
                                using namespace v8;
                                Local<Promise::Resolver> resolve = Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();
                                global_resolver.Reset(isolate, resolve);
                                printf("++++++++++ return promise\n");
                                info.GetReturnValue().Set(resolve->GetPromise());
                            })
                            ->GetFunction(context)
                            .ToLocalChecked())
                .Check();
        }
        // Enter the context for compiling and running the hello world script.
        v8::Context::Scope context_scope(context);
        {
            // Create a string containing the JavaScript source code.
            v8::Local<v8::String> source = v8::String::NewFromUtf8Literal(isolate, R"__(
                function Fuck() {
                    this.content = "fuck u google"
                    this.times = 114514 
                }
                Fuck.name = 'fuck js'
                let obj = new Fuck()
                my_print(`type: typeof(obj)`)
                my_print(`constructor: ${obj.constructor}`)

                async function test_async() {
                    let call_value = await wait_call()
                    my_print("+++++++++++++ wait call: call_value")
                }
                test_async()

                let test = { content: "shit", times: 1, fuck_func: Fuck }
                test.fuck_func()
                my_print(`content: ${test.content}, times: ${test.times}`)

                sb = Symbol("sb")
                test[sb] = function() {
                    my_print(`call sb [${this.content} + ${this.times}]`)
                }
                test[sb]()

                class MyClass {
                    constructor() {
                        my_print(`=====> type: ${typeof(new.target)}`)
                        my_print(`=====> name: ${new.target.name}`)
                        my_print(`=====> static value: ${new.target.aaa}`)
                    }

                    static aaa = "fuck u google"
                }
                my_cls = new MyClass();
                
                {
                    let new_data = new MyData()
                    my_print(`'${new_data.content}' ${new_data.times} times`)
                    let new_data2 = new MyData("fuck")
                    my_print(`'${new_data2.content}' ${new_data2.times} times`)
                }
                )__");
            // Compile the source code.
            v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
            // Run the script to get the result.
            v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
            // Convert the result to an UTF8 string and print it.
            v8::String::Utf8Value utf8(isolate, result);
            printf("%s\n", *utf8);

            [[maybe_unused]] bool done = global_resolver.Get(isolate)->Resolve(context, v8::String::NewFromUtf8Literal(isolate, "fuck u google")).ToChecked();
            global_resolver.Reset();

            // context->GetMicrotaskQueue()->PerformCheckpoint(isolate);
        }

        isolate->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);
        // GC 通知 isolate->LowMemoryNotification()
        // 等待 GC isolate->IdleNotificationDeadline()

        my_data_instances.clear();
    }
    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    my_data_instances.clear();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}

// test type
struct TestType {
    TestType() { SKR_LOG_FMT_INFO(u8"call ctor"); }
    TestType(int32_t v)
        : value(v)
    {
        SKR_LOG_FMT_INFO(u8"call ctor with param {}", v);
    }
    int32_t value;
};
SKR_RTTR_TYPE(TestType, "9f7696d7-4c20-4b11-84eb-7124b666c56e");
SKR_EXEC_STATIC_CTOR
{
    using namespace skr::rttr;
    register_type_loader(type_id_of<TestType>(), [](Type* type) {
        // init type
        type->init(ETypeCategory::Record);

        // build type
        RecordBuilder<TestType> builder{ &type->record_data() };
        builder
            .basic_info()
            .ctor<uint32_t>()
            .field<&TestType::value>(u8"value");
    });
};

int main(int argc, char* argv[])
{
    using namespace skr::v8;

    V8Isolate isolate;
    V8Context context(&isolate);

    // init
    init_v8();
    isolate.init();
    context.init();

    // import types
    isolate.make_record_template(skr::rttr::type_of<TestType>());

    // inject into context
    context.install_templates();

    // exec code
    context.exec_script(u8R"__(
        let test = new TestType()
        let test_with_value_ctor = new TestType(114514);
    )__");

    // shutdown
    context.shutdown();
    isolate.shutdown();
    shutdown_v8();
}