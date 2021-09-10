![dzr logo](.github/.logo.svg)

# DZR: the command line deezer.com player

> ⚠️ For [legal reasons](https://github.com/github/dmca/blob/master/2021/02/2021-02-10-deezer.md) this project
> - does not come with any [track decryption key](https://github.com/yne/dzr/wiki)
> - does not read/store (even temporary) any file on your machine

# Preview
[![asciicast](https://asciinema.org/a/406758.svg)](https://asciinema.org/a/406758)

# Install
- Find and save the [DZR_CBC key](https://github.com/yne/dzr/wiki)
- Install dependencies: `curl` `jq` `dialog` `mpv` `openssl` (or `openssl-tool` in Android)
- Install dzr itself: `curl -sL github.com/yne/dzr/archive/master.tar.gz | tar xzf -` (this command will save dzr into a `dzr-master` folder)

> The Deezer API rapidly change, so update `dzr` when something goes wrong

# Compatibility

This project shall work on Linux, *BSD and Android (via [F-droid Termux](https://termux.com/)) but if you need more OS support, feel free to open an issue (or a PR if it's MacOS/Windows)
