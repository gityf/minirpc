/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CRpcServer.
*/
#ifndef _COMMON_RPC_RPC_PB_h
#define _COMMON_RPC_RPC_PB_H_
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>

#define RPCREGI(x, y)                                \
    ::google::protobuf::Service *service = new y();  \
    x->AddService(service);

#define RPCREGI_BASIC(x, y, z)                        \
    ::google::protobuf::Service *service = new y(z); \
    x.AddService(service);

// library must have one extern 'C' function named 
typedef int (*FuncPtr_LibEntry)(void *aRpcServer);
#endif // _COMMON_RPC_RPC_PB_H_
