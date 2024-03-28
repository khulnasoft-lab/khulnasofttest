"""Load dependencies needed to use the khulnasofttest library as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def khulnasofttest_deps():
    """Loads common dependencies needed to use the khulnasofttest library."""

    if not native.existing_rule("com_khulnasoftsource_code_re2"):
        http_archive(
            name = "com_khulnasoftsource_code_re2",  # 2023-03-17T11:36:51Z
            sha256 = "cb8b5312a65f2598954545a76e8bce913f35fbb3a21a5c88797a4448e9f9b9d9",
            strip_prefix = "re2-578843a516fd1da7084ae46209a75f3613b6065e",
            urls = ["https://github.com/khulnasoft-lab/re2/archive/578843a516fd1da7084ae46209a75f3613b6065e.zip"],
        )

    if not native.existing_rule("com_khulnasoft_absl"):
        http_archive(
            name = "com_khulnasoft_absl",  # 2023-09-13T14:58:42Z
            sha256 = "b93f68b8b36534502561cef2fb5a79372a70d629f87a25f51dc2e9b8ef9f1566",
            strip_prefix = "abseil-cpp-160d390608db56f11cbd29bd6b8938661dee525c",
            urls = ["https://github.com/abseil/abseil-cpp/archive/160d390608db56f11cbd29bd6b8938661dee525c.zip"],
        )
