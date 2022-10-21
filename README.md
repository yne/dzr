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

## Compatibility

- Linux and {Free,Open}BSD
- Android (using [Termux](https://termux.com/) from F-droid)
- Window 10 (running dzr as CGI server from WSL and browsing http://127.0.0.1:8000 from Windows)

## Install

### From the AUR (Arch Linux)

```sh
yay -S dzr
```

### From [GURU](https://github.com/gentoo/guru) (Gentoo)

```sh
emerge --ask dzr
```

### From sources

Save source into a `dzr-master` folder, then copy into /usr/local/bin :

```bash
curl -sL github.com/yne/dzr/archive/master.tar.gz | tar xzf -
sudo mv dzr-master/dzr* /usr/local/bin
```

## Usage Examples

```sh
dzr             # welcome screen
dzr /artist/860 # browse deezer.com/en/artist/860
```

## Automatic ID3v2 Tagging

Use `dzr-id3` to rename (as `$ARTIST - $TITLE.mp3`) and tag a given MP3 using it deezer track id

```sh
# the following examples are all equivalent
dzr-id3 1043317462 daylight.mp3
dzr-id3 /track/1043317462 daylight.mp3
dzr-id3 https://deezer.com/en/track/1043317462 daylight.mp3
```

## Real time Lyrics

Use `dzr-srt` to extract lyrics of the current track and pass it to mpv as --sub-file :

```sh
PLAYER='mpv --sub-file=<(dzr-srt $id) -' dzr /track/14408104
```

## HTTP/Web interface

In addition to it command line interface, `dzr` also support being invoked from a cgi server :

```sh
mkdir -p cgi-bin
cp dzr* ./cgi-bin/
python3 -m http.server --cgi
```

You shall then be able to play any track over HTTP (ex: http://127.0.0.1:8000/cgi-bin/dzr?6113114 )

A **basic** web interface is also available on http://127.0.0.1:8000

Feel free to create your own frontend an publish it as a new repository (not as a dzr fork) with the [dzr](https://github.com/topics/dzr) tag.
