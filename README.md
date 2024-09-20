![dzr logo](.github/.logo.svg)

# DZR: the command line deezer.com player

## Features

- Cross-platform support: Linux, *BSD, MacOS, Android, Windows+WSL
- Little dependencies: `curl`, `jq`, `dialog`, `openssl` (`openssl-tool` in Android)
- Real-time Lyrics display
- Web interface support (see [dzr](https://github.com/topics/dzr)-tagged frontend)
- ID3v2 tag injector from Deezer metadata (cover, artist, ...)
- Play without storing/caching on your machine for [legal reasons](https://github.com/github/dmca/blob/master/2021/02/2021-02-10-deezer.md)
- No private deezer key in the source (auto-extracted from web player, also for legal reasons)
- VSCode extension [VSIX](https://github.com/yne/dzr/releases) experimental port

## Preview (CLI)

[![asciicast](https://asciinema.org/a/406758.svg)](https://asciinema.org/a/406758)

## Preview (VSIX)

![Screenshot](https://github.com/yne/dzr/assets/5113053/37b6cd26-8876-4d77-92bb-293ff248e21d)

## Install

| Platform | command | version |
|----------|---------|---------|
| MacOS + [brew](https://formulae.brew.sh/formula/dzr)       | `brew install dzr` | ![](https://repology.org/badge/version-for-repo/homebrew/dzr.svg?header=)
| Arch Linux + [AUR](https://aur.archlinux.org/packages/dzr) | `yay -S dzr`       | ![](https://repology.org/badge/version-for-repo/aur/dzr.svg?header=)
| Gentoo + [GURU](https://github.com/gentoo/guru)            | `emerge --ask dzr` | ![](https://repology.org/badge/version-for-repo/gentoo_ovl_guru/dzr.svg?header=)
| Ubuntu + [Snap](https://snapcraft.io/dzr) | `snap install --edge dzr` | [Help Me](https://github.com/yne/dzr/issues/25)
| Linux + [Flatpak](https://www.flatpak.org/) | `flatpak install dzr` | [Help Me](https://github.com/yne/dzr/issues/25)
| Nix + [Flake](https://wiki.nixos.org/wiki/flakes) | `nix run github.com/yne/dzr` | [Help Me](https://github.com/yne/dzr/issues/25)
| Android + [Termux](https://f-droid.org/packages/com.termux/) | `curl -sL github.com/yne/dzr/archive/master.tar.gz \| tar xzf -` <br> `mv dzr-master/dzr* $PREFIX/bin` | [![](https://img.shields.io/badge/-tar.gz-40c010?logo=hackthebox)](https://github.com/yne/dzr/archive/master.tar.gz)
| VSCode | `code --install-extension ./path/to/dzr-*.vsix` | [![](https://img.shields.io/badge/VSIX-4c1?logo=visualstudiocode)](https://github.com/yne/dzr/releases)

## Usage

```sh
# browse api.deezer.com
dzr

# browse a specific api.deezer.com url
dzr /artist/860

# play a specific track
dzr /track/1043317462

# use a custom PLAYER (mpg123 v1.31+ is a lightweight alternative)
PLAYER="mpg123 -" dzr

# inject deezer ID3v2 into MP3 (require eyeD3) and rename it as $ARTIST - $TITLE.mp3
dzr-id3 https://deezer.com/track/1043317462 tagme.mp3

# show track lyrics as srt
dzr-srt https://deezer.com/track/14408104

# play track with it lyrics
PLAYER='dzr-srt $id > .srt ; mpv --sub-file=.srt -' dzr /track/14408104

# play track with it srt (using non-POSIX compliant process substitution)
PLAYER='mpv --sub-file=<(dzr-srt $id) -' dzr /track/14408104

# install dzr into ./cgi-bin/. Then serve it
mkdir -p ./cgi-bin/ && install dzr* ./cgi-bin/
python3 -m http.server --cgi
open http://127.0.0.1:8000/cgi-bin/dzr?6113114
```
