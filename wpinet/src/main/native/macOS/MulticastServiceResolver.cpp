// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#if defined(__APPLE__)

#include "wpinet/MulticastServiceResolver.h"

#include <netinet/in.h>
#include <poll.h>

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <wpi/SmallVector.h>

#include "ResolverThread.h"
#include "dns_sd.h"

#include "sys/event.h"

using namespace wpi;

struct DnsResolveState {
  DnsResolveState(MulticastServiceResolver::Impl* impl,
                  std::string_view serviceNameView)
      : pImpl{impl} {
    data.serviceName = serviceNameView;
  }

  MulticastServiceResolver::Impl* pImpl;
  MulticastServiceResolver::ServiceData data;
};

struct MulticastServiceResolver::Impl {
  std::string serviceType;
  MulticastServiceResolver* resolver;
  std::shared_ptr<ResolverThread> thread = ResolverThread::Get();
  std::vector<std::unique_ptr<DnsResolveState>> resolveStates;
  DNSServiceRef serviceRef = nullptr;
  dnssd_sock_t threadShutdownHandle;

  void removeState(DnsResolveState* context) {
    std::erase_if(resolveStates,
                  [context](auto& a) { return a.get() == context; });
  }

  void onFound(ServiceData&& data) {
    resolver->PushData(std::forward<ServiceData>(data));
  }
};

MulticastServiceResolver::MulticastServiceResolver(
    std::string_view serviceType) {
  pImpl = std::make_unique<Impl>();
  pImpl->serviceType = serviceType;
  pImpl->resolver = this;
}

MulticastServiceResolver::~MulticastServiceResolver() noexcept {
  Stop();
}

static void ServiceGetAddrInfoReply(DNSServiceRef sdRef, DNSServiceFlags flags,
                                    uint32_t interfaceIndex,
                                    DNSServiceErrorType errorCode,
                                    const char* hostname,
                                    const struct sockaddr* address,
                                    uint32_t ttl, void* context) {
  // We're done with the service ref. Deallocate it
  DNSServiceRefDeallocate(sdRef);

  DnsResolveState* resolveState = static_cast<DnsResolveState*>(context);

  if (errorCode != kDNSServiceErr_NoError) {
    resolveState->pImpl->removeState(resolveState);
    return;
  }

  resolveState->data.hostName = hostname;
  resolveState->data.ipv4Address =
      reinterpret_cast<const struct sockaddr_in*>(address)->sin_addr.s_addr;

  resolveState->pImpl->onFound(std::move(resolveState->data));

  resolveState->pImpl->removeState(resolveState);
}

void ServiceResolveReply(DNSServiceRef sdRef, DNSServiceFlags flags,
                         uint32_t interfaceIndex, DNSServiceErrorType errorCode,
                         const char* fullname, const char* hosttarget,
                         uint16_t port, /* In network byte order */
                         uint16_t txtLen, const unsigned char* txtRecord,
                         void* context) {
  DNSServiceRefDeallocate(sdRef);

  DnsResolveState* resolveState = static_cast<DnsResolveState*>(context);

  if (errorCode != kDNSServiceErr_NoError) {
    resolveState->pImpl->removeState(resolveState);
    return;
  }

  resolveState->data.port = ntohs(port);

  int txtCount = TXTRecordGetCount(txtLen, txtRecord);
  char keyBuf[256];
  uint8_t valueLen;
  const void* value;

  for (int i = 0; i < txtCount; i++) {
    errorCode = TXTRecordGetItemAtIndex(txtLen, txtRecord, i, sizeof(keyBuf),
                                        keyBuf, &valueLen, &value);
    if (errorCode == kDNSServiceErr_NoError) {
      if (valueLen == 0) {
        // No value
        resolveState->data.txt.emplace_back(
            std::pair<std::string, std::string>{std::string{keyBuf}, {}});
      } else {
        resolveState->data.txt.emplace_back(std::pair<std::string, std::string>{
            std::string{keyBuf},
            std::string{reinterpret_cast<const char*>(value), valueLen}});
      }
    }
  }

  DNSServiceRef copyRef = resolveState->pImpl->serviceRef;

  errorCode = DNSServiceGetAddrInfo(
      &copyRef, kDNSServiceFlagsShareConnection, interfaceIndex,
      kDNSServiceProtocol_IPv4, hosttarget, ServiceGetAddrInfoReply, context);

  if (errorCode != kDNSServiceErr_NoError) {
    resolveState->pImpl->removeState(resolveState);
  }
}

static void DnsCompletion(DNSServiceRef sdRef, DNSServiceFlags flags,
                          uint32_t interfaceIndex,
                          DNSServiceErrorType errorCode,
                          const char* serviceName, const char* regtype,
                          const char* replyDomain, void* context) {
  // Don't free the browse ref, its the default one.

  if (errorCode != kDNSServiceErr_NoError) {
    return;
  }
  if (!(flags & kDNSServiceFlagsAdd)) {
    return;
  }

  MulticastServiceResolver::Impl* impl =
      static_cast<MulticastServiceResolver::Impl*>(context);

  DNSServiceRef copyRef = impl->serviceRef;

  auto& resolveState = impl->resolveStates.emplace_back(
      std::make_unique<DnsResolveState>(impl, serviceName));

  errorCode = DNSServiceResolve(
      &copyRef, kDNSServiceFlagsShareConnection, interfaceIndex, serviceName,
      regtype, replyDomain, ServiceResolveReply, resolveState.get());

  if (errorCode != kDNSServiceErr_NoError) {
    impl->removeState(resolveState.get());
  }
}

bool MulticastServiceResolver::HasImplementation() const {
  return true;
}

void MulticastServiceResolver::Start() {
  if (pImpl->serviceRef) {
    return;
  }

  DNSServiceErrorType status = DNSServiceCreateConnection(&pImpl->serviceRef);
  if (status != kDNSServiceErr_NoError) {
    return;
  }

  // Fire up a browse
  DNSServiceRef copyRef = pImpl->serviceRef;

  status = DNSServiceBrowse(&copyRef, kDNSServiceFlagsShareConnection, 0,
                            pImpl->serviceType.c_str(), "local", DnsCompletion,
                            pImpl.get());
  if (status != kDNSServiceErr_NoError) {
    // Because the thread never starts, we need to clean up the service ref.
    DNSServiceRefDeallocate(pImpl->serviceRef);
    pImpl->serviceRef = nullptr;
    return;
  }

  auto shutdownHandle = pImpl->thread->AddServiceRef(pImpl->serviceRef);
  if (!shutdownHandle.has_value()) {
    // Because the thread never starts, we need to clean up the service ref.
    DNSServiceRefDeallocate(pImpl->serviceRef);
    pImpl->serviceRef = nullptr;
  }

  pImpl->threadShutdownHandle = *shutdownHandle;
}

void MulticastServiceResolver::Stop() {
  if (!pImpl->serviceRef) {
    return;
  }

  // RemoveServiceRef will block until its actually removed.
  // It will deallocate the service ref too.
  pImpl->thread->RemoveServiceRef(pImpl->threadShutdownHandle, pImpl->serviceRef);
  pImpl->serviceRef = nullptr;
  pImpl->resolveStates.clear();
}

#endif  // defined(__APPLE__)
