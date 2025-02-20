// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the overhead of getting a certificate.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/certificates.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

using namespace Azure::Core::_internal;

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Certificates {
        namespace Test {

  /**
   * @brief A test to measure getting a certificate performance.
   *
   */
  class GetCertificate : public Azure::Perf::PerfTest {
  private:
    std::string m_vaultUrl;
    std::string m_certificateName;
    std::string m_tenantId;
    std::string m_clientId;
    std::string m_secret;
    std::shared_ptr<Azure::Identity::ClientSecretCredential> m_credential;
    std::unique_ptr<Azure::Security::KeyVault::Certificates::CertificateClient> m_client;

  public:
    /**
     * @brief Get the Ids and secret
     *
     */
    void Setup() override
    {
      m_vaultUrl = m_options.GetOptionOrDefault<std::string>(
          "vaultUrl", Environment::GetVariable("AZURE_KEYVAULT_URL"));
      m_tenantId = m_options.GetOptionOrDefault<std::string>(
          "TenantId", Environment::GetVariable("AZURE_TENANT_ID"));
      m_clientId = m_options.GetOptionOrDefault<std::string>(
          "ClientId", Environment::GetVariable("AZURE_CLIENT_ID"));
      m_secret = m_options.GetOptionOrDefault<std::string>(
          "Secret", Environment::GetVariable("AZURE_CLIENT_SECRET"));
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          m_tenantId, m_clientId, m_secret);
      m_client = std::make_unique<Azure::Security::KeyVault::Certificates::CertificateClient>(
          m_vaultUrl,
          m_credential,
          InitClientOptions<Azure::Security::KeyVault::Certificates::CertificateClientOptions>());
      this->CreateRandomNameCertificate();
    }

    /**
     * @brief Create a random named certificate.
     *
     */
    void CreateRandomNameCertificate()
    {
      std::string name("perf");
      int suffixLen = 10;
      static const char alphanum[]
          = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
      std::string suffix;
      suffix.reserve(suffixLen);

      for (int i = 0; i < suffixLen; ++i)
      {
        suffix += alphanum[rand() % (sizeof(alphanum) - 1)];
      }

      m_certificateName = name + suffix;
      CertificateCreateOptions options;
      options.Policy.Subject = "CN=xyz";
      options.Policy.ValidityInMonths = 12;
      options.Policy.Enabled = true;

      options.Properties.Enabled = true;
      options.Properties.Name = m_certificateName;
      options.Policy.ContentType = CertificateContentType::Pkcs12;
      options.Policy.IssuerName = "Self";

      LifetimeAction action;
      action.LifetimePercentage = 80;
      action.Action = CertificatePolicyAction::AutoRenew;
      options.Policy.LifetimeActions.emplace_back(action);
      auto duration = std::chrono::minutes(5);
      auto deadline = std::chrono::system_clock::now() + duration;
      Azure::Core::Context context;
      auto response = m_client->StartCreateCertificate(
          m_certificateName, options, context.WithDeadline(deadline));
      auto pollResult = response.PollUntilDone(std::chrono::milliseconds(2000));
    }

    /**
     * @brief Construct a new GetCertificate test.
     *
     * @param options The test options.
     */
    GetCertificate(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      auto t = m_client->GetCertificate(m_certificateName);
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {
          {"vaultUrl", {"--vaultUrl"}, "The Key Vault Account.", 1, false},
          {"TenantId", {"--tenantId"}, "The tenant Id for the authentication.", 1, false},
          {"ClientId", {"--clientId"}, "The client Id for the authentication.", 1, false},
          {"Secret", {"--secret"}, "The secret for authentication.", 1, false, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "GetCertificate", "Get a certificate", [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Security::KeyVault::Certificates::Test::GetCertificate>(
                options);
          }};
    }
  };

}}}}} // namespace Azure::Security::KeyVault::Certificates::Test
