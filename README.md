![dzr logo](.github/.logo.svg)

# DZR: Stream from SHell

> ⚠️ For [legal reasons](https://github.com/github/dmca/blob/master/2021/02/2021-02-10-deezer.md), dzr does not come with any [track decryption key](https://github.com/yne/dzr/wiki) and so, does not work "out of the box"  

# Preview
[![asciicast](https://asciinema.org/a/406758.svg)](https://asciinema.org/a/406758)

# Install/Update
- Find (or just google) the [DZR_CBC key](https://github.com/yne/dzr/wiki) then `export` it in your `~/.profile`
- Install deps: `curl` `jq` `openssl` `dialog` `mpv` (package names may vary depending on your OS)
- Install dzr: `rm -f dzr* && wget raw.githubusercontent.com/yne/dzr/master/dzr{,-dec,-url} && chmod +x ./dzr*`

> The Deezer API rapidly change so in case of breakage: update your dzr or open an issue

# Compatibility

This project shall work on any OS:
- Linux
- *BSD (even OpenBSD)
- Android via [F-Droid Termux](https://termux.com/) (Note: `pkg install openssl-tool` instead of `openssl`)
- Open an Issue/PR if you need more OS