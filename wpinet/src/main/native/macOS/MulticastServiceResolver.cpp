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

  ~DnsResolveState() {
    if (resolveRef) {
      DNSServiceRefDeallocate(resolveRef);
    }
    if (addrInfoRef) {
      DNSServiceRefDeallocate(addrInfoRef);
    }
  }

  MulticastServiceResolver::Impl* pImpl{nullptr};
  MulticastServiceResolver::ServiceData data;
  DNSServiceRef resolveRef{nullptr};
  DNSServiceRef addrInfoRef{nullptr};
};

struct MulticastServiceResolver::Impl {
  std::string serviceType;
  MulticastServiceResolver* resolver;
  std::shared_ptr<ResolverThread> thread = ResolverThread::Get();
  std::vector<std::unique_ptr<DnsResolveState>> resolveStates;
  DNSServiceRef serviceRef = nullptr;
  DNSServiceRef globalServiceRef = nullptr;

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

  resolveState->addrInfoRef = resolveState->pImpl->globalServiceRef;

  errorCode = DNSServiceGetAddrInfo(
      &resolveState->addrInfoRef, kDNSServiceFlagsShareConnection,
      interfaceIndex, kDNSServiceProtocol_IPv4, hosttarget,
      ServiceGetAddrInfoReply, context);

  if (errorCode != kDNSServiceErr_NoError) {
    resolveState->addrInfoRef = nullptr;
    resolveState->pImpl->removeState(resolveState);
  }
}

static void DnsCompletion(DNSServiceRef sdRef, DNSServiceFlags flags,
                          uint32_t interfaceIndex,
                          DNSServiceErrorType errorCode,
                          const char* serviceName, const char* regtype,
                          const char* replyDomain, void* context) {
  if (errorCode != kDNSServiceErr_NoError) {
    return;
  }
  if (!(flags & kDNSServiceFlagsAdd)) {
    return;
  }

  MulticastServiceResolver::Impl* impl =
      static_cast<MulticastServiceResolver::Impl*>(context);

  auto& resolveState = impl->resolveStates.emplace_back(
      std::make_unique<DnsResolveState>(impl, serviceName));

  resolveState->resolveRef = impl->globalServiceRef;

  errorCode = DNSServiceResolve(&resolveState->resolveRef,
                                kDNSServiceFlagsShareConnection, interfaceIndex,
                                serviceName, regtype, replyDomain,
                                ServiceResolveReply, resolveState.get());

  if (errorCode != kDNSServiceErr_NoError) {
    // If errored, clear out the resolve ref, and then clear out the resolver
    resolveState->resolveRef = nullptr;
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

  // Fire up a browse
  pImpl->thread->AddServiceRef([this](DNSServiceRef serviceRef) -> void {
    pImpl->serviceRef = serviceRef;
    pImpl->globalServiceRef = serviceRef;
    DNSServiceErrorType status = DNSServiceBrowse(
        &pImpl->serviceRef, kDNSServiceFlagsShareConnection, 0,
        pImpl->serviceType.c_str(), "local", DnsCompletion, pImpl.get());
    if (status != kDNSServiceErr_NoError) {
      pImpl->serviceRef = nullptr;
    }
  });
  return;
}

void MulticastServiceResolver::Stop() {
  if (!pImpl->serviceRef) {
    return;
  }

  // RemoveServiceRef will block until its actually removed.
  // It will deallocate the service ref too.
  pImpl->thread->RemoveServiceRef([this] {
    DNSServiceRefDeallocate(pImpl->serviceRef);
    pImpl->resolveStates.clear();
  });
  pImpl->serviceRef = nullptr;
}

#endif  // defined(__APPLE__)
