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

#include "update_callback_stub.h"
#include "securec.h"
#include "update_helper.h"

using namespace std;

namespace OHOS {
namespace update_engine {
int32_t UpdateCallbackStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        ENGINE_LOGI("UpdateCallbackStub ReadInterfaceToken fail");
        return -1;
    }
    switch (code) {
        case CHECK_VERSION: {
            VersionInfo checkVersionInfo;
            UpdateHelper::ReadVersionInfo(data, checkVersionInfo);
            OnCheckVersionDone(checkVersionInfo);
            break;
        }
        case DOWNLOAD: {
            Progress downloadInfo;
            UpdateHelper::ReadUpdateProgress(data, downloadInfo);
            OnDownloadProgress(downloadInfo);
            break;
        }
        case UPGRADE: {
            Progress upgradeInfo;
            UpdateHelper::ReadUpdateProgress(data, upgradeInfo);
            OnUpgradeProgress(upgradeInfo);
            break;
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return 0;
}
}
}
