workspace(name = "com_khulnasoft_khulnasofttest")

load("//:khulnasofttest_deps.bzl", "khulnasofttest_deps")
khulnasofttest_deps()

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
  name = "rules_python",  # 2023-07-31T20:39:27Z
  sha256 = "1250b59a33c591a1c4ba68c62e95fc88a84c334ec35a2e23f46cbc1b9a5a8b55",
  strip_prefix = "rules_python-e355becc30275939d87116a4ec83dad4bb50d9e1",
  urls = ["https://github.com/bazelbuild/rules_python/archive/e355becc30275939d87116a4ec83dad4bb50d9e1.zip"],
)

http_archive(
  name = "bazel_skylib",  # 2023-05-31T19:24:07Z
  sha256 = "57aac84d14e659cfc8c91d400b62a127b16a8a330cc5745c8ac70a3227b5d3cf",
  strip_prefix = "bazel-skylib-1a1ee6c2308fc474ddb57c24e0aa82c58c11de1b",
  urls = ["https://github.com/bazelbuild/bazel-skylib/archive/1a1ee6c2308fc474ddb57c24e0aa82c58c11de1b.zip"],
)

http_archive(
  name = "platforms",  # 2023-07-28T19:44:27Z
  sha256 = "40eb313613ff00a5c03eed20aba58890046f4d38dec7344f00bb9a8867853526",
  strip_prefix = "platforms-4ad40ef271da8176d4fc0194d2089b8a76e19d7b",
  urls = ["https://github.com/bazelbuild/platforms/archive/4ad40ef271da8176d4fc0194d2089b8a76e19d7b.zip"],
)
