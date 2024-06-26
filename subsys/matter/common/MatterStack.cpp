/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "MatterStack.h"
#include "AppTask.h"
#include "BLEManagerImpl.h"
#include "FabricTableDelegate.h"

#include <DeviceInfoProviderImpl.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>

#include <zephyr/fs/nvs.h>
#include <zephyr/settings/settings.h>

using namespace ::chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace ::chip::DeviceLayer;

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace
{
// Test network key
const uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
#define VerifyInitResultOrReturn(ec, msg)                                                          \
	VerifyOrReturn(ec == CHIP_NO_ERROR,                                                        \
		       LOG_ERR(msg " [Error: %d]", Instance().sInitResult.Format()))
} // namespace

void MatterStack::matter_internal_init()
{
	Instance().sInitResult = ThreadStackMgr().InitThreadStack();
	VerifyInitResultOrReturn(Instance().sInitResult,
				 "ThreadStackMgr().InitThreadStack() failed");

#if CONFIG_CHIP_THREAD_SSED
	Instance().sInitResult = ConnectivityMgr().SetThreadDeviceType(
		ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice);
#elif CONFIG_OPENTHREAD_MTD_SED
	Instance().sInitResult = ConnectivityMgr().SetThreadDeviceType(
		ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else
	Instance().sInitResult = ConnectivityMgr().SetThreadDeviceType(
		ConnectivityManager::kThreadDeviceType_Router);
#endif
	VerifyInitResultOrReturn(Instance().sInitResult, "SetThreadDeviceType fail");

#if CONFIG_CHIP_FACTORY_DATA
	Instance().sInitResult = mFactoryDataProvider.Init();
	VerifyInitResultOrReturn(Instance().sInitResult, "FactoryDataProvider::Init() failed");
	SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
	SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
	SetCommissionableDataProvider(&mFactoryDataProvider);
	// Read EnableKey from the factory data.
	MutableByteSpan enableKey(sTestEventTriggerEnableKey);
	err = mFactoryDataProvider.GetEnableKey(enableKey);
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("mFactoryDataProvider.GetEnableKey() failed. Could not delegate a test "
			"event trigger");
		memset(sTestEventTriggerEnableKey, 0, sizeof(sTestEventTriggerEnableKey));
	}
#else
	SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());
	SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

	// Init ZCL Data Model and start server
	static CommonCaseDeviceServerInitParams initParams;
	static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
	static OTATestEventTriggerHandler sOtaTestEventTriggerHandler{};
	Instance().sInitResult =
		sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey));
	VerifyInitResultOrReturn(Instance().sInitResult, "Tesat trigger delegate init fail");
	Instance().sInitResult = sTestEventTriggerDelegate.AddHandler(&sOtaTestEventTriggerHandler);
	VerifyInitResultOrReturn(Instance().sInitResult,
				 "OTa test event trigger handlertrigger addfail");
	(void)initParams.InitializeStaticResourcesBeforeServerInit();
	initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
	Instance().sInitResult = chip::Server::GetInstance().Init(initParams);
	VerifyInitResultOrReturn(Instance().sInitResult, "Server init fail");
	AppFabricTableDelegate::Init();

	ConfigurationMgr().LogDeviceConfig();
	PrintOnboardingCodes(
		chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

	PlatformMgr().AddEventHandler(Instance().ChipEventHandler, reinterpret_cast<intptr_t>(this));

	if (Instance().dev_init_cb) {
		Instance().sInitResult = Instance().dev_init_cb();
		VerifyInitResultOrReturn(Instance().sInitResult, "Device post init fail");
	}
}

void MatterStack::InitInternal(intptr_t class_ptr)
{

	MatterStack *entry = reinterpret_cast<MatterStack *>(class_ptr);

	entry->matter_internal_init();
	entry->signal_condition();
}

void MatterStack::ChipEventHandler(const ChipDeviceEvent *event, intptr_t arg)
{
	MatterStack *entry = reinterpret_cast<MatterStack *>(arg);

	switch (event->Type) {
	case DeviceEventType::kCHIPoBLEAdvertisingChange:
		entry->sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
		LOG_INF("BLE connection state %d", Instance().sHaveBLEConnections);
		break;

	case DeviceEventType::kDnssdInitialized:
		LOG_INF("DNS init done");
		break;

	case DeviceEventType::kDnssdRestartNeeded:
		LOG_INF("DNS Restart needed");
		break;

	case DeviceEventType::kThreadConnectivityChange:
		LOG_INF("Thread connectivity change: %d", event->ThreadConnectivityChange.Result);
		if (event->ThreadConnectivityChange.Result == kConnectivity_Established) {
			LOG_INF("Thread connectivity established");
		} else if (event->ThreadConnectivityChange.Result == kConnectivity_Lost) {
			LOG_INF("Thread connectivity lost");
		}
		break;

	case DeviceEventType::kThreadStateChange:
		entry->sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
		entry->sIsThreadEnabled = ConnectivityMgr().IsThreadEnabled();
		entry->sIsThreadAttached = ConnectivityMgr().IsThreadAttached();
		LOG_INF("Thread State Provisioned %d, enabled %d, Atteched %d",
			entry->sIsThreadProvisioned, entry->sIsThreadEnabled,
			entry->sIsThreadAttached);
		break;

	case DeviceEventType::kCommissioningComplete:
		LOG_INF("Commission complet node ide %lld, fabid %d",
			event->CommissioningComplete.nodeId,
			event->CommissioningComplete.fabricIndex);
		break;

	case DeviceEventType::kServiceProvisioningChange:
		LOG_INF("Service Provisioned %d, conf update %d",
			event->ServiceProvisioningChange.IsServiceProvisioned,
			event->ServiceProvisioningChange.ServiceConfigUpdated);
		break;
	case DeviceEventType::kFailSafeTimerExpired:
		LOG_INF("Commission fail safe timer expiration: fab id %d, NoCinvoked %d, "
			"updNoCCmd %d",
			event->FailSafeTimerExpired.fabricIndex,
			event->FailSafeTimerExpired.addNocCommandHasBeenInvoked,
			event->FailSafeTimerExpired.updateNocCommandHasBeenInvoked);
		break;

	default:
		LOG_INF("Unhandled event types: %d", event->Type);
		break;
	}
}

CHIP_ERROR MatterStack::matter_stack_init(DevInit device_init_cb)
{
	CHIP_ERROR err = CHIP_NO_ERROR;
	k_mutex_lock(&Instance().sInitMutex, K_FOREVER);
	dev_init_cb = device_init_cb;

	err = chip::Platform::MemoryInit();
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("MemoryInit fail");
		return err;
	}

	err = PlatformMgr().InitChipStack();
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("InitChipStack fail");
		return err;
	}
	// Shedule Init
	return PlatformMgr().ScheduleWork(InitInternal, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR MatterStack::matter_stack_start()
{
	CHIP_ERROR err = PlatformMgr().StartEventLoopTask();
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("PlatformMgr().StartEventLoopTask() failed");
		return err;
	}
	wait_condition();
	return sInitResult;
}
