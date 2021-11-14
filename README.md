![dzr logo](.github/.logo.svg)

# DZR: the command line deezer.com player

> ⚠️ For [legal reasons](https://github.com/github/dmca/blob/master/2021/02/2021-02-10-deezer.md) this project
> - does not contain the [track decryption key](https://github.com/yne/dzr/wiki)
> - does not cache any tracks on your machine

# Preview
[![asciicast](https://asciinema.org/a/406758.svg)](https://asciinema.org/a/406758)

# Dependencies

- `mpv` for playback (because of `PLAYER="mpv -"` default env variable)
- `curl` for HTTP query
- `jq` for API parsing
- `dialog` for TUI
- `openssl` (or `openssl-tool` in Android) for track decryption

# Install
Save into a `dzr-master` folder :

```bash
curl -sL github.com/yne/dzr/archive/master.tar.gz | tar xzf -
```

Optional: move all `dzr*` scripts in a `*/bin` folder of your `$PATH` so you can just type `dzr` to use it.

# Compatibility

This project has been tested on:
- Linux (Ubuntu 18.04/20.04 but other distrib shall work)
- OpenBSD (But any BSD shall work)
- Android (using [Termux](https://termux.com/) from F-droid)

Need more OS support ? Open an issue or a pull request.

