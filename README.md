<img width=100% height=200 src=.logo.svg>
<p align=center>Your favorite flows, straight from shell</p>

> ⚠️ For legal reasons, `dzr` does not contain any private keys,
> so [find them](https://github.com/yne/dzr/wiki) yourself.  

# Preview

<p align=center><a href="https://asciinema.org/a/NpET2MMpGN41QW2a0JOjFru0l">
<img height=200 src="https://asciinema.org/a/NpET2MMpGN41QW2a0JOjFru0l.svg">
</a></p>

# Download

See [release](https://github.com/yne/dzr/releases) for the prebuild binary.

# Build

```bash
cc dzr.c # HTTP only, work with DZR_API method
```

<details><summary>Build with HTTPS support via LibSSL</summary>

```bash
apt install gcc libssl1* libssl-dev
gcc dzr.c -DUSE_SSL -lssl -lcrypto -o dzr
```

</details>

# Setup

Setup `dzr` environement variables using the
[wiki](https://github.com/yne/dzr/wiki) in your `.bashrc`/`.zshrc`.

# Usage

```sh
dzr 997764 > my.mp3
dzr 997764 | mpv -
DZR_FMT=9 dzr 997764 > my.flac
dzr-api artist pink floyds | xargs dzr | mpv -
```

