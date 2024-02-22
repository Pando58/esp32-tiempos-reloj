{ pkgs ? import <nixpkgs> { } }: let
  #
# in (pkgs.buildFHSEnv {
#   name = "arduino-env";
#
#   targetPkgs = pkgs: (with pkgs; [
#     ncurses
#     arduino
#     arduino-cli
#     zlib
#     (python3.withPackages(ps: [
#       ps.pyserial
#     ]))
#   ]);
#
#   multiPkgs = null;
# }).env
in pkgs.mkShell {
  buildInputs = with pkgs; [
    arduino-cli
    arduino-language-server
    (python3.withPackages(ps: [
      ps.pyserial
    ]))
    picocom
  ];
}
