// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "wpi/mDNSResolver.h"

#include "AvahiClient.h"
#include "wpi/SmallString.h"
#include "wpi/mutex.h"

using namespace wpi;

struct mDNSResolver::Impl {
  AvahiFunctionTable& table = AvahiFunctionTable::Get();
  std::shared_ptr<AvahiThread> thread = AvahiThread::Get();
  AvahiClient* client;
  AvahiServiceBrowser* browser;
  std::string serviceType;
  mDnsRevolveCompletionFunc onFound;
};

mDNSResolver::mDNSResolver(std::string_view serviceType,
                           mDnsRevolveCompletionFunc onFound) {
  pImpl = std::make_unique<Impl>();
  pImpl->serviceType = serviceType;
  pImpl->onFound = std::move(onFound);
}

mDNSResolver::~mDNSResolver() noexcept {
  Stop();
}

static void ResolveCallback(AvahiServiceResolver* r, AvahiIfIndex interface,
                            AvahiProtocol protocol, AvahiResolverEvent event,
                            const char* name, const char* type,
                            const char* domain, const char* host_name,
                            const AvahiAddress* address, uint16_t port,
                            AvahiStringList* txt, AvahiLookupResultFlags flags,
                            void* userdata) {
  mDNSResolver::Impl* impl = reinterpret_cast<mDNSResolver::Impl*>(userdata);

  if (event == AVAHI_RESOLVER_FOUND) {
    if (address->proto == AVAHI_PROTO_INET) {
      AvahiStringList* strLst = txt;
      std::vector<std::pair<std::string, std::string>> txtArr;
      while (strLst != nullptr) {
        std::string_view value{reinterpret_cast<const char*>(strLst->text),
                               strLst->size};
        strLst = strLst->next;
        size_t splitIndex = value.find('=');
        if (splitIndex == value.npos) {
          // Todo make this just do key
          continue;
        }
        std::string_view key = value.substr(0, splitIndex);
        value = value.substr(splitIndex + 1, value.size() - splitIndex - 1);
        txtArr.emplace_back(std::pair<std::string, std::string>{key, value});
      }
      wpi::SmallString<256> outputHostName;
      char label[256];
      do {
        impl->table.unescape_label(&host_name, label, sizeof(label));
        if (label[0] == '\0') {
          break;
        }
        outputHostName.append(label);
        outputHostName.append(".");
      } while (true);

      impl->onFound(address->data.ipv4.address, port, name, outputHostName.str(),
                    txtArr);
    }
  }

  impl->table.service_resolver_free(r);
}

static void BrowseCallback(AvahiServiceBrowser* b, AvahiIfIndex interface,
                           AvahiProtocol protocol, AvahiBrowserEvent event,
                           const char* name, const char* type,
                           const char* domain, AvahiLookupResultFlags flags,
                           void* userdata) {
  mDNSResolver::Impl* impl = reinterpret_cast<mDNSResolver::Impl*>(userdata);

  if (event == AVAHI_BROWSER_NEW) {
    impl->table.service_resolver_new(
        impl->table.service_browser_get_client(b), interface, protocol, name,
        type, domain, AVAHI_PROTO_UNSPEC, AVAHI_LOOKUP_USE_MULTICAST,
        ResolveCallback, userdata);
  }
}

void mDNSResolver::Start() {
  if (!pImpl->table.IsValid()) {
    return;
  }
  std::scoped_lock lock{*pImpl->thread};
  if (pImpl->client) {
    return;
  }

  pImpl->client =
      pImpl->table.client_new(pImpl->thread->GetPoll(), AVAHI_CLIENT_NO_FAIL,
                              nullptr, nullptr, nullptr);

  pImpl->browser = pImpl->table.service_browser_new(
      pImpl->client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
      pImpl->serviceType.c_str(), "local",
      AvahiLookupFlags::AVAHI_LOOKUP_USE_MULTICAST, BrowseCallback,
      pImpl.get());
}

void mDNSResolver::Stop() {
  if (!pImpl->table.IsValid()) {
    return;
  }
  std::scoped_lock lock{*pImpl->thread};
  if (pImpl->client) {
    pImpl->table.client_free(pImpl->client);
    pImpl->client = nullptr;
  }
}