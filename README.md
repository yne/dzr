![dzr logo](.github/.logo.svg)

# DZR: the command line deezer.com player

> ⚠️ For [legal reasons](https://github.com/github/dmca/blob/master/2021/02/2021-02-10-deezer.md) this project
> - does not contain any track decryption key
> - does not cache any tracks on your machine

## Preview
[![asciicast](https://asciinema.org/a/406758.svg)](https://asciinema.org/a/406758)

## Dependencies

- `mpv` for playback (because of `PLAYER="mpv -"` default env variable)
- `curl` for HTTP query
- `jq` for API parsing
- `dialog` for TUI
- `openssl` (or `openssl-tool` in Android) for track decryption

## Install

### From the AUR (Arch Linux)

```sh
yay -S dzr
```

### From [GURU](https://github.com/gentoo/guru) (Gentoo)

```sh
emerge --ask dzr
```

### Manually

Save into a `dzr-master` folder :

```bash
curl -sL github.com/yne/dzr/archive/master.tar.gz | tar xzf -
```

Optional: move all `dzr*` scripts in a `*/bin` folder of your `$PATH` so you can just type `dzr` to use it.

## Usage

Run `dzr` without argument to get to the home screen

```sh
dzr
```

### From URL

Run `dzr` with a valid url, to browse this artist, album, playlist, track.

Example: `deezer.com/en/artist/860` become `dzr /artist/860`

### As CGI

`dzr` can serve tracks over HTTP when running as [CGI](https://en.wikipedia.org/wiki/Common_Gateway_Interface)

```sh
# create a cgi-bin directory
mkdir cgi-bin
# copy all your dzr* executables into this directory
cp /path/to/your/dzr* cgi-bin
# run a basic HTTP server with CGI support
python3 -m http.server --cgi
# you shall now be able to play from HTTP
mpv http://0.0.0.0:8000/cgi-bin/dzr?6113114
```

## Compatibility

This project has been tested on:
- Linux (Ubuntu 18.04/20.04 but other distrib shall work)
- OpenBSD (But any BSD shall work)
- Android (using [Termux](https://termux.com/) from F-droid)

Need more OS support ? Open an issue or a pull request.
