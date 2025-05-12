{
  description = "Open5GS fully‑sandboxed devShell with every Meson dependency";

  inputs = {
    nixpkgs.url     = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            # build system
            meson                    # Meson build front‑end :contentReference[oaicite:2]{index=2}
            ninja                    # Ninja backend :contentReference[oaicite:3]{index=3}
            cmake                    # for prometheus-client-c subproject :contentReference[oaicite:4]{index=4}
            pkg-config               # pkg‑config discovery :contentReference[oaicite:5]{index=5}
            git                      # for version detection
            flex                     # freeDiameter needs flex :contentReference[oaicite:6]{index=6}
            bison                    # freeDiameter needs bison :contentReference[oaicite:7]{index=7}

            # compiler & LSP
            llvmPackages.clang       # clang compiler
            llvmPackages.clang-tools # clangd, clang‑tidy

            # core Open5GS C deps
            talloc                   # hierarchical allocator :contentReference[oaicite:8]{index=8}
            mongoc                   # libmongoc‑1.0 (mongo‑c‑driver) :contentReference[oaicite:9]{index=9}
            libyaml                  # yaml‑0.1 :contentReference[oaicite:10]{index=10}
            libmicrohttpd            # libmicrohttpd :contentReference[oaicite:11]{index=11}
            gnutls                   # gnutls :contentReference[oaicite:12]{index=12}
            openssl                  # libssl, libcrypto :contentReference[oaicite:13]{index=13}
            nghttp2                  # libnghttp2 :contentReference[oaicite:14]{index=14}
            curl                     # libcurl :contentReference[oaicite:15]{index=15}

            # supplemental networking libs
            usrsctp                  # userland SCTP :contentReference[oaicite:16]{index=16}
            libtins                  # packet‑crafting for UPF :contentReference[oaicite:17]{index=17}

            # security / auth
            libgcrypt                # gcrypt :contentReference[oaicite:18]{index=18}
            libidn                   # idn :contentReference[oaicite:19]{index=19}
            cyrus_sasl               # SASL support for MongoDB :contentReference[oaicite:20]{index=20}
          ];

          # ensure compile_commands.json ends up under build/
          shellHook = ''
            export MESON_BUILD_ROOT=build
          '';
        };
      });
}
