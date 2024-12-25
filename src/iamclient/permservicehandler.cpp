/*
 * Copyright (C) 2024 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iamclient/permservicehandler.hpp"
#include "logger/logmodule.hpp"
#include "pbconvert/common.hpp"

namespace aos::common::iamclient {

/***********************************************************************************************************************
 * Public
 **********************************************************************************************************************/

Error PermissionsServiceHandler::Init(
    const std::string& IAMProtectedServerURL, const std::string& certStorage, TLSCredentialsItf& TLSCredentials)
{
    LOG_INF() << "Initializing permissions service handler: IAMProtectedServerURL=" << IAMProtectedServerURL.c_str()
              << ", certStorage=" << certStorage.c_str();

    mCertStorage           = certStorage;
    mTLSCredentials        = &TLSCredentials;
    mIAMProtectedServerURL = IAMProtectedServerURL;

    return ErrorEnum::eNone;
}

RetWithError<StaticString<iam::permhandler::cSecretLen>> PermissionsServiceHandler::RegisterInstance(
    const InstanceIdent&                                                          instanceIdent,
    [[maybe_unused]] const Array<iam::permhandler::FunctionalServicePermissions>& instancePermissions)
{
    LOG_INF() << "Register instance: serviceID=" << instanceIdent.mServiceID
              << ", subjectID=" << instanceIdent.mSubjectID << ", instance=" << instanceIdent.mInstance;

    try {
        auto ctx = std::make_unique<grpc::ClientContext>();
        ctx->set_deadline(std::chrono::system_clock::now() + cIAMPermissionsServiceTimeout);

        auto stub = iamanager::v5::IAMPermissionsService::NewStub(
            grpc::CreateCustomChannel(mIAMProtectedServerURL, CreateCredentials(), grpc::ChannelArguments()));

        auto request = pbconvert::ConvertToProto(instanceIdent, instancePermissions);

        iamanager::v5::RegisterInstanceResponse response;

        if (auto status = stub->RegisterInstance(ctx.get(), request, &response); !status.ok()) {
            return {StaticString<iam::permhandler::cSecretLen>(), ErrorEnum::eRuntime};
        }

        return {response.secret().c_str(), ErrorEnum::eNone};

    } catch (const std::exception& e) {
        return {StaticString<iam::permhandler::cSecretLen>(), Error(ErrorEnum::eRuntime, e.what())};
    }
}

Error PermissionsServiceHandler::UnregisterInstance(const InstanceIdent& instanceIdent)
{
    LOG_INF() << "Unregister instance: serviceID=" << instanceIdent.mServiceID
              << ", subjectID=" << instanceIdent.mSubjectID << ", instance=" << instanceIdent.mInstance;

    try {

        auto ctx = std::make_unique<grpc::ClientContext>();
        ctx->set_deadline(std::chrono::system_clock::now() + cIAMPermissionsServiceTimeout);

        auto stub = iamanager::v5::IAMPermissionsService::NewStub(
            grpc::CreateCustomChannel(mIAMProtectedServerURL, CreateCredentials(), grpc::ChannelArguments()));

        iamanager::v5::UnregisterInstanceRequest request;
        request.mutable_instance()->CopyFrom(pbconvert::ConvertToProto(instanceIdent));

        google::protobuf::Empty response;

        if (auto status = stub->UnregisterInstance(ctx.get(), request, &response); !status.ok()) {
            return ErrorEnum::eRuntime;
        }

        return ErrorEnum::eNone;
    } catch (const std::exception& e) {
        return Error(ErrorEnum::eRuntime, e.what());
    }
}

Error PermissionsServiceHandler::GetPermissions([[maybe_unused]] const String& secret,
    [[maybe_unused]] const String& funcServerID, [[maybe_unused]] InstanceIdent& instanceIdent,
    [[maybe_unused]] Array<iam::permhandler::PermKeyValue>& servicePermissions)
{
    LOG_DBG() << "Get permissions: serviceID=" << instanceIdent.mServiceID << ", subjectID=" << instanceIdent.mSubjectID
              << ", instance=" << instanceIdent.mInstance;

    return ErrorEnum::eNotSupported;
}

/***********************************************************************************************************************
 * Private
 **********************************************************************************************************************/

std::shared_ptr<grpc::ChannelCredentials> PermissionsServiceHandler::CreateCredentials()
{
    iam::certhandler::CertInfo certInfo;

    auto [credential, err] = mTLSCredentials->GetMTLSClientCredentials(mCertStorage.c_str());
    if (!err.IsNone()) {
        throw std::runtime_error("failed to get MTLS config");
    }

    return credential;
}

} // namespace aos::common::iamclient