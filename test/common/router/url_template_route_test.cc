#include "envoy/config/route/v3/route.pb.h"
#include "envoy/config/route/v3/route.pb.validate.h"

#include "common/common/assert.h"
#include "common/router/config_impl.h"

#include "test/mocks/server/instance.h"
#include "test/mocks/stream_info/mocks.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy::Router {
namespace {

using envoy::config::route::v3::DirectResponseAction;
using envoy::config::route::v3::Route;
using envoy::config::route::v3::RouteConfiguration;
using envoy::config::route::v3::RouteMatch;
using envoy::config::route::v3::VirtualHost;
using testing::NiceMock;
using testing::ReturnRef;

/**
 * Generates a request with the path:
 * - /shelves/shelf_x/route_x
 */
static Http::TestRequestHeaderMapImpl genRequestHeaders(int route_num) {
  return Http::TestRequestHeaderMapImpl{
      {":authority", "www.google.com"},
      {":method", "GET"},
      {":path", absl::StrCat("/shelves/shelf_", route_num, "/route_", route_num)},
      {"x-forwarded-proto", "http"}};
}

/**
 * Generates the route config for the type of matcher being tested.
 */
static RouteConfiguration genRouteConfig() {
  // Create the base route config.
  RouteConfiguration route_config;
  VirtualHost* v_host = route_config.add_virtual_hosts();
  v_host->set_name("default");
  v_host->add_domains("*");

  for (int i = 0; i < 1; ++i) {
    Route* route = v_host->add_routes();
    DirectResponseAction* direct_response = route->mutable_direct_response();
    direct_response->set_status(200);
    RouteMatch* match = route->mutable_match();

    match->set_url_template(absl::StrCat("/shelves/{shelf_id}/route_", i));
    // TODO: Uncomment these lines to test re2.
//    regex->mutable_google_re2();
//    regex->set_regex(absl::StrCat("^/shelves/[^\\\\/]+/route_", i, "$"));
  }

  return route_config;
}

TEST(UrlTemplateRouteTest, A) {
  // Setup router for benchmarking.
  Api::ApiPtr api = Api::createApiForTest();
  NiceMock<Server::Configuration::MockServerFactoryContext> factory_context;
  NiceMock<Envoy::StreamInfo::MockStreamInfo> stream_info;
  ON_CALL(factory_context, api()).WillByDefault(ReturnRef(*api));

  // Create router config.
  ConfigImpl config(genRouteConfig(), factory_context,
                    ProtobufMessage::getNullValidationVisitor(), true);

  config.route(genRequestHeaders(0), stream_info, 0);
}

}
}  // namespace Envoy::Router
