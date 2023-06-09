/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cJSON.h"
#include "if_system_ability_manager.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "iupdate_callback.h"
#include "iupdate_service.h"
#include "parameters.h"
#include "unittest_comm.h"
#include "update_callback.h"
#include "update_callback_proxy.h"
#include "update_helper.h"
#include "update_service.h"
#include "update_service_kits_impl.h"
#include "update_service_proxy.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::update_engine;
static const int32_t DLNOW_10 = 10;
static const int32_t DLNOW_20 = 20;
static const int32_t DLNOW_50 = 50;
static const int32_t DLNOW_70 = 70;
static const int32_t DLNOW_80 = 80;
static const int32_t DLTOTAL_NUM = 100;
static const int32_t UPGRADE_INTERVAL_2 = 2222;
static const int32_t UPGRADE_INTERVAL_3 = 3333;
static const int32_t JSON_MAX_SIZE = 4096;
#define UNUSED(x) (void)(x)
static UpdateService *GetUpdateService()
{
    static UpdateService *updateServer = nullptr;
    if (updateServer == nullptr) {
        updateServer = new UpdateService(0, true);
    }
    return updateServer;
}

class UpdateServerTest : public ::testing::Test {
public:
    UpdateServerTest()
    {
        updateServer_ = GetUpdateService();
        updateCallBack_ = new UpdateCallback();
    }
    virtual ~UpdateServerTest()
    {
        delete updateCallBack_;
        updateCallBack_ = nullptr;
    }

    void SetUp() {}
    void TearDown() {}
    void TestBody() {}

    int TestRegisterCallback()
    {
        return 0;
    }

    int TestCheckNewVersion()
    {
        ENGINE_CHECK(updateServer_ != nullptr, return -1, "Can not find server");
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::CHECK_VERSION, inData, reply, msgOption);
        return 0;
    }

    int TestDownload()
    {
        ENGINE_CHECK(updateServer_ != nullptr, return -1, "Can not find server");
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::DOWNLOAD, inData, reply, msgOption);
        EXPECT_EQ(0, reply.ReadInt32());
        sleep(1);
        updateServer_->OnRemoteRequest(IUpdateService::CANCEL, inData, reply, msgOption);
        return 0;
    }

    int TestDoUpdate()
    {
        ENGINE_CHECK(updateServer_ != nullptr, return -1, "Can not find server");
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::UPGRADE, inData, reply, msgOption);
        EXPECT_EQ(-1, reply.ReadInt32());
        sptr<IUpdateCallback> callback_ {};
        UpdateContext ctx {};
        ctx.upgradeFile = "/data/updater/updater/updater.zip";
        UpdateService *updateServer = new UpdateService(0, true);
        int ret = updateServer->RegisterUpdateCallback(ctx, callback_);
        EXPECT_NE(updateServer, nullptr);
        updateServer->DoUpdate();
        ret = updateServer->UnregisterUpdateCallback();
        EXPECT_EQ(0, ret);
        delete updateServer;
        return 0;
    }

    int TestGetUpdatePolicy(UpdatePolicy &policy)
    {
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::GET_POLICY, inData, reply, msgOption);
        UpdateHelper::ReadUpdatePolicy(reply, policy);
        return 0;
    }

    int TestUpdatePolicy()
    {
        ENGINE_CHECK(updateServer_ != nullptr, return -1, "Can not find server");
        UpdatePolicy policy;
        policy.autoDownload = true;
        policy.autoDownloadNet = true;
        policy.mode = INSTALLMODE_AUTO;
        policy.autoUpgradeInterval[0] = UPGRADE_INTERVAL_2;
        policy.autoUpgradeInterval[1] = UPGRADE_INTERVAL_3;
        MessageParcel inData;
        UpdateHelper::WriteUpdatePolicy(inData, policy);
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::SET_POLICY, inData, reply, msgOption);
        UpdatePolicy policy2;
        (void)TestGetUpdatePolicy(policy2);
        EXPECT_EQ(policy.autoUpgradeInterval[0], policy2.autoUpgradeInterval[0]);
        return 0;
    }

    int TestGetNewVersion()
    {
        VersionInfo versionInfo;
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::GET_NEW_VERSION, inData, reply, msgOption);
        UpdateHelper::ReadVersionInfo(reply, versionInfo);
        EXPECT_EQ(versionInfo.status, HAS_NEW_VERSION);
        return 0;
    }

    int TestGetUpgradeStatus()
    {
        UpgradeInfo info;
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::GET_STATUS, inData, reply, msgOption);
        UpdateHelper::ReadUpgradeInfo(reply, info);
        return 0;
    }

    int TestRebootAndClean()
    {
        VersionInfo versionInfo;
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::REBOOT_CLEAN, inData, reply, msgOption);
        EXPECT_EQ(reply.ReadInt32(), 0);
        return 0;
    }

    int TestRebootAndInstall()
    {
        VersionInfo versionInfo;
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::REBOOT_INSTALL, inData, reply, msgOption);
        EXPECT_EQ(reply.ReadInt32(), 0);
        return 0;
    }

    int TestRegisterUpdateCallback()
    {
        VersionInfo versionInfo;
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::REGISTER_CALLBACK, inData, reply, msgOption);
        EXPECT_EQ(reply.ReadInt32(), -1);
        return 0;
    }

    int TestUnRegisterUpdateCallback()
    {
        VersionInfo versionInfo;
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateServer_->OnRemoteRequest(IUpdateService::UNREGISTER_CALLBACK, inData, reply, msgOption);
        EXPECT_EQ(reply.ReadInt32(), 0);
        return 0;
    }

    int TestReadCheckVersiondescriptInfo()
    {
        cJSON *temp = cJSON_CreateObject();
        EXPECT_NE(temp, nullptr);
        cJSON_AddStringToObject(temp, "descriptPackageId", "descriptPackageId1");
        cJSON_AddStringToObject(temp, "content", "content1");
        cJSON_AddNumberToObject(temp, "id", 1);
        cJSON* array = cJSON_CreateArray();
        EXPECT_NE(array, nullptr);
        cJSON_AddItemToArray(array, temp);

        VersionInfo info {};
        updateServer_->ReadCheckVersiondescriptInfo(array, info);
        EXPECT_STREQ(info.descriptInfo[0].descriptPackageId.c_str(), "descriptPackageId1");
        EXPECT_STREQ(info.descriptInfo[0].content.c_str(), "content1");
        return 0;
    }

    int TestUpdateServiceProxy()
    {
        sptr<ISystemAbilityManager> systemMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        EXPECT_NE(systemMgr, nullptr);
        sptr<IRemoteObject> remoteObj = systemMgr->GetSystemAbility(UPDATE_DISTRIBUTED_SERVICE_ID);
        EXPECT_NE(remoteObj, nullptr);
        UpdateServiceProxy *proxy = new UpdateServiceProxy(remoteObj);
        UpdateCallbackProxy *updateCallBack = new UpdateCallbackProxy(remoteObj);
        std::string miscFile = "/dev/block/platform/soc/10100000.himci.eMMC/by-name/misc";
        std::string packageName = "/data/updater/updater/updater.zip";
        const std::string cmd = "--update_package=/data/updater/updater.zip";
        VersionInfo info {};
        Progress progress {};
        UpdateContext ctx {};
        updateCallBack->OnCheckVersionDone(info);
        updateCallBack->OnDownloadProgress(progress);
        updateCallBack->OnUpgradeProgress(progress);
        proxy->RegisterUpdateCallback(ctx, updateCallBack);
        proxy->UnregisterUpdateCallback();
        proxy->DoUpdate();
        proxy->RebootAndClean(miscFile, cmd);
        proxy->RebootAndInstall(miscFile, packageName);
        EXPECT_EQ(info.status, HAS_NEW_VERSION);
        EXPECT_EQ(progress.status, UPDATE_STATE_INIT);
        delete proxy;
        return 0;
    }

    int TestUpdateCallbackCheckNewVersion()
    {
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateCallBack_->OnRemoteRequest(IUpdateService::CHECK_VERSION, inData, reply, msgOption);
        EXPECT_EQ(reply.ReadInt32(), 0);
        return 0;
    }

    int TestUpdateCallbackDownload()
    {
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateCallBack_->OnRemoteRequest(IUpdateService::DOWNLOAD, inData, reply, msgOption);
        EXPECT_EQ(reply.ReadInt32(), 0);
        return 0;
    }

    int TestUpdateCallbackUpgrade()
    {
        MessageParcel inData;
        MessageParcel reply;
        MessageOption msgOption;
        updateCallBack_->OnRemoteRequest(IUpdateService::UPGRADE, inData, reply, msgOption);
        EXPECT_EQ(reply.ReadInt32(), 0);
        return 0;
    }
    void GetServerIp(std::string &ip)
    {
        ip = OHOS::system::GetParameter("update.serverip", "127.0.0.1");
    }

    int TestDownLoadProgress()
    {
        DownloadThread *download = new DownloadThread(
            [&](const std::string &filaName, const Progress &progress) -> int {
            return 0;
        });
        EXPECT_EQ(download == 0, 0);
        std::string serverAddr;
        GetServerIp(serverAddr);
        std::string url = "http://";
        url += serverAddr;
        url += "/updater.zip";
        download->StartDownload("updater.zip", url);
        DownloadThread::DownloadProgress(download, DLTOTAL_NUM, DLNOW_10, 0, 0);
        DownloadThread::DownloadProgress(download, DLTOTAL_NUM, DLNOW_20, 0, 0);
        DownloadThread::DownloadProgress(download, DLTOTAL_NUM, DLNOW_50, 0, 0);
        DownloadThread::DownloadProgress(download, DLTOTAL_NUM, DLNOW_70, 0, 0);
        download->StopDownload();
        DownloadThread::DownloadProgress(download, DLTOTAL_NUM, DLNOW_80, 0, 0);
        DownloadThread::DownloadProgress(download, DLTOTAL_NUM, DLTOTAL_NUM, 0, 0);

        // get file
        std::string jsonFileName = TEST_PATH_FROM;
        jsonFileName += "packageInfos.json";
        size_t length = DownloadThread::GetLocalFileLength(jsonFileName);
        // write file
        FILE *downloadFile = fopen("packageInfos.json", "ab+");
        if (downloadFile != nullptr) {
            VersionInfo versionInfo {};
            std::vector<char> buffer(JSON_MAX_SIZE);
            fread(buffer.data(), 1, JSON_MAX_SIZE, downloadFile);
            UpdateService::ParseJsonFile(buffer, versionInfo);
            DownloadThread::WriteFunc(reinterpret_cast<void *>buffer.data(), length, 1, downloadFile);
        }
        delete download;
        return 0;
    }

    int TestNewUpdateService()
    {
        UpdateService *updateServer = new UpdateService(0, true);
        updateServer->OnStart();
        updateServer->OnStop();
        delete updateServer;
        return 0;
    }

private:
    UpdateService *updateServer_ = nullptr;
    UpdateCallback *updateCallBack_ = nullptr;
};

class MockUpdateServiceKits : public UpdateServiceKits {
public:
    MockUpdateServiceKits() = default;
    virtual ~MockUpdateServiceKits() = default;

    int32_t RegisterUpdateCallback(const UpdateContext &ctx, const UpdateCallbackInfo &cb) override
    {
        UNUSED(ctx);
        UNUSED(cb);
        return 0;
    }

    int32_t UnregisterUpdateCallback() override
    {
        return 0;
    }

    int32_t CheckNewVersion() override
    {
        return 0;
    }

    int32_t DownloadVersion() override
    {
        return 0;
    }

    int32_t DoUpdate() override
    {
        return 0;
    }

    int32_t GetNewVersion(VersionInfo &versionInfo) override
    {
        UNUSED(versionInfo);
        return 0;
    }

    int32_t GetUpgradeStatus(UpgradeInfo &info) override
    {
        UNUSED(info);
        return 0;
    }

    int32_t SetUpdatePolicy(const UpdatePolicy &policy) override
    {
        UNUSED(policy);
        return 0;
    }

    int32_t GetUpdatePolicy(UpdatePolicy &policy) override
    {
        UNUSED(policy);
        return 0;
    }

    int32_t Cancel(int32_t service) override
    {
        return service;
    }

    virtual int32_t RebootAndClean(const std::string &miscFile, const std::string &cmd)
    {
        UNUSED(miscFile);
        UNUSED(cmd);
        return 0;
    }

    virtual int32_t RebootAndInstall(const std::string &miscFile, const std::string &packageName)
    {
        UNUSED(miscFile);
        UNUSED(packageName);
        return 0;
    }
};

HWTEST_F(UpdateServerTest, TestDownload, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestCheckNewVersion();
    (void)test.TestDownload();
}

HWTEST_F(UpdateServerTest, TestDoUpdate, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestDoUpdate();
}

HWTEST_F(UpdateServerTest, TestGetNewVersion, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestCheckNewVersion();
    (void)test.TestGetNewVersion();
}

HWTEST_F(UpdateServerTest, TestUpdatePolicy, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestUpdatePolicy();
}

HWTEST_F(UpdateServerTest, TestGetUpgradeStatus, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestGetUpgradeStatus();
}

HWTEST_F(UpdateServerTest, TestRebootAndClean, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestRebootAndClean();
}

HWTEST_F(UpdateServerTest, TestRebootAndInstall, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestRebootAndInstall();
}

HWTEST_F(UpdateServerTest, TestRegisterUpdateCallback, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestRegisterUpdateCallback();
}

HWTEST_F(UpdateServerTest, TestUnRegisterUpdateCallback, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestUnRegisterUpdateCallback();
}

HWTEST_F(UpdateServerTest, TestReadCheckVersiondescriptInfo, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestReadCheckVersiondescriptInfo();
}

HWTEST_F(UpdateServerTest, TestUpdateCallbackCheckNewVersion, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestUpdateCallbackCheckNewVersion();
}

HWTEST_F(UpdateServerTest, TestUpdateCallbackDownload, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestUpdateCallbackDownload();
}

HWTEST_F(UpdateServerTest, TestUpdateCallbackUpgrade, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestUpdateCallbackUpgrade();
}

HWTEST_F(UpdateServerTest, TestDownLoadProgress, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestDownLoadProgress();
}

HWTEST_F(UpdateServerTest, TestNewUpdateService, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestNewUpdateService();
}

HWTEST_F(UpdateServerTest, TestUpdateServiceProxy, TestSize.Level1)
{
    UpdateServerTest test;
    (void)test.TestUpdateServiceProxy();
}

HWTEST_F(UpdateServerTest, TestKitsResetService, TestSize.Level1)
{
    sptr<ISystemAbilityManager> systemMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(systemMgr, nullptr);
    sptr<IRemoteObject> remoteObj = systemMgr->GetSystemAbility(UPDATE_DISTRIBUTED_SERVICE_ID);
    EXPECT_NE(remoteObj, nullptr);
    sptr<IRemoteObject::DeathRecipient> deathRecipient = new UpdateServiceKitsImpl::DeathRecipient();
    EXPECT_NE(deathRecipient, nullptr);
    deathRecipient->OnRemoteDied(remoteObj);
    UpdateService *saServer = new (std::nothrow)UpdateService(0, true);
    EXPECT_NE(saServer, nullptr);
    const std::string miscFile = "/dev/block/platform/soc/10100000.himci.eMMC/by-name/misc";
    const std::string packageName = "--update_package=/data/updater/updater.zip";
    int32_t ret = saServer->RebootAndInstall(miscFile, packageName);
    EXPECT_EQ(ret, 0);
    delete saServer;
}

HWTEST_F(UpdateServerTest, TestKitsRemoteCallbackOnCheckVersionDone, TestSize.Level1)
{
    VersionInfo info {};
    UpdateCallbackInfo callBackFun {};
    sptr<UpdateServiceKitsImpl::RemoteUpdateCallback> remoteCallBack =
        new (std::nothrow)UpdateServiceKitsImpl::RemoteUpdateCallback(callBackFun);
    EXPECT_NE(remoteCallBack, nullptr);
    remoteCallBack->OnCheckVersionDone(info);
}

HWTEST_F(UpdateServerTest, TestKitsRemoteCallbackOnDownloadProgress, TestSize.Level1)
{
    Progress progress {};
    UpdateCallbackInfo callBackFun {};
    sptr<UpdateServiceKitsImpl::RemoteUpdateCallback> remoteCallBack =
        new (std::nothrow)UpdateServiceKitsImpl::RemoteUpdateCallback(callBackFun);
    EXPECT_NE(remoteCallBack, nullptr);
    remoteCallBack->OnDownloadProgress(progress);
}

HWTEST_F(UpdateServerTest, TestKitsRemoteCallbackOnUpgradeProgress, TestSize.Level1)
{
    Progress progress {};
    UpdateCallbackInfo callBackFun {};
    sptr<UpdateServiceKitsImpl::RemoteUpdateCallback> remoteCallBack =
        new (std::nothrow)UpdateServiceKitsImpl::RemoteUpdateCallback(callBackFun);
    EXPECT_NE(remoteCallBack, nullptr);
    remoteCallBack->OnUpgradeProgress(progress);
}

HWTEST_F(UpdateServerTest, TestKitsRegisterUpdateCallback, TestSize.Level1)
{
    UpdateCallbackInfo callBackFun {};
    UpdateContext ctx {};
    int ret = UpdateServiceKits::GetInstance().RegisterUpdateCallback(ctx, callBackFun);
    EXPECT_EQ(ret, 0);
    ret = UpdateServiceKits::GetInstance().UnregisterUpdateCallback();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(UpdateServerTest, TestKitsDoUpdate, TestSize.Level1)
{
    int ret = UpdateServiceKits::GetInstance().DoUpdate();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(UpdateServerTest, TestKitsCancel, TestSize.Level1)
{
    int32_t service = 0;
    int ret = UpdateServiceKits::GetInstance().Cancel(service);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(UpdateServerTest, TestKitsRebootAndClean, TestSize.Level1)
{
    const std::string miscFile = "/dev/block/platform/soc/10100000.himci.eMMC/by-name/misc";
    const std::string cmd = "--update_package=/data/updater/updater.zip";
    int ret = UpdateServiceKits::GetInstance().RebootAndClean(miscFile, cmd);
    EXPECT_NE(ret, 0);
}

HWTEST_F(UpdateServerTest, TestKitsReadDataFromSSL, TestSize.Level1)
{
    int32_t engineSocket = 10;
    UpdateService *updateServer = new (std::nothrow)UpdateService(0, true);
    EXPECT_NE(updateServer, nullptr);
    updateServer->ReadDataFromSSL(engineSocket);
    delete updateServer;
}

HWTEST_F(UpdateServerTest, TestKits, TestSize.Level1)
{
    UpdateServiceKits *kit = new (std::nothrow)MockUpdateServiceKits();
    EXPECT_NE(kit, nullptr);
    delete kit;
}

HWTEST_F(UpdateServerTest, TestUpdateServiceKitsImpl, TestSize.Level1)
{
    std::string miscFile = "/dev/block/platform/soc/10100000.himci.eMMC/by-name/misc";
    std::string packageName = "/data/updater/updater/updater.zip";
    int ret = UpdateServiceKits::GetInstance().RebootAndInstall(miscFile, packageName);
    printf("RebootAndInstall: %d\n", ret);
}

HWTEST_F(UpdateServerTest, TestUpdateServiceRegisterUpdateCallback, TestSize.Level1)
{
    UpdateContext ctx {};
    UpdateService *updateServer = new (std::nothrow)UpdateService(0, true);
    EXPECT_NE(updateServer, nullptr);
    updateServer->RegisterUpdateCallback(ctx, nullptr);
    delete updateServer;
}

HWTEST_F(UpdateServerTest, TestVerifyDownloadPkg, TestSize.Level1)
{
    Progress downloadProgress {};
    std::string path = "/data/updater/updater/test.txt";
    UpdateService *updateServer = new (std::nothrow)UpdateService(0, true);
    EXPECT_NE(updateServer, nullptr);
    updateServer->VerifyDownloadPkg(path, downloadProgress);
    delete updateServer;
}
