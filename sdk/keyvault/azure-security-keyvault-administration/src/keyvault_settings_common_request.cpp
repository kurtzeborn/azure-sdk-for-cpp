// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/keyvault_settings_common_request.hpp"
#include "private/administration_constants.hpp"

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <memory>

using namespace Azure::Security::KeyVault;
using namespace Azure::Core::Http::_internal;
using namespace Azure::Security::KeyVault::Administration::_detail;

std::unique_ptr<Azure::Core::Http::RawResponse> KeyVaultSettingsCommonRequest::SendRequest(
    Azure::Core::Http::_internal::HttpPipeline const& pipeline,
    Azure::Core::Http::Request& request,
    Azure::Core::Context const& context)
{
  auto response = pipeline.Send(request, context);
  auto responseCode = response->GetStatusCode();

  switch (responseCode)
  {

    // 200, 201, 202, 204 are accepted responses
    case Azure::Core::Http::HttpStatusCode::Ok:
    case Azure::Core::Http::HttpStatusCode::Created:
    case Azure::Core::Http::HttpStatusCode::Accepted:
    case Azure::Core::Http::HttpStatusCode::NoContent:
      break;
    default:
      throw Azure::Core::RequestFailedException(response);
  }
  return response;
}

Azure::Core::Http::Request KeyVaultSettingsCommonRequest::CreateRequest(
    Azure::Core::Url url,
    std::string const& apiVersion,
    Azure::Core::Http::HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content)
{
  using namespace Azure::Core::Http;
  Request request = content == nullptr ? Request(method, url) : Request(method, url, content);

  request.SetHeader(ContentHeaderName, ApplicationJsonValue);
  request.GetUrl().AppendQueryParameter(ApiVersionQueryParamName, apiVersion);

  for (std::string const& p : path)
  {
    if (!p.empty())
    {
      request.GetUrl().AppendPath(p);
    }
  }
  return request;
}
