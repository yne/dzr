<img width=100% height=200 src=.logo.svg>
<p align=center>Your favorite flows, straight from shell</p>

> ⚠️ For legal reasons, `dzr` does contain any encryption/decryption keys,  
> so you must [find them](https://github.com/yne/dzr/wiki) by yourself.  
> If you think this project break any legal rules: open an issue to discuss

# Preview

<p align=center><a href="https://asciinema.org/a/NpET2MMpGN41QW2a0JOjFru0l">
<img height=200 src="https://asciinema.org/a/NpET2MMpGN41QW2a0JOjFru0l.svg">
</a></p>

# Download

See [release](https://github.com/yne/dzr/releases) for the prebuild binary.

You can also build it yourself:

```bash
apt install gcc libssl1* libssl-dev # build deps
gcc dzr.c -lssl -lcrypto -o dzr
```
# Setup

Be sure to have set the `DZR_AES` and `DZR_CBC` environement variables.

* **I want to use my account:**
you shall also set your `DZR_SID`.  
* **I don't have/want an account:**
set the `DZR_LUT` to point to the `dzr-db` binary (see bellow).

Once complete, you should have the following variable set in your `.bashrc`:

  - `DZR_AES` = `jo..............` [HOWTO](https://github.com/yne/dzr/wiki)
  - `DZR_CBC` = `g4el............` [HOWTO](https://github.com/yne/dzr/wiki)
  - `DZR_LUT` = `/path/to/dzr-db %s`
  - `DZR_SID` = your_sid (optional) [HOWTO](https://github.com/yne/dzr/wiki)
  - `DZR_FMT` = `0` (can be: 128Kb:`0`, 320Kb:`3`, AAC96:`8`, FLAC:`9`)

# Usage

```sh
dzr 997764 > my.mp3
dzr 997764 | mpv --cache-secs 360 -
DZR_FMT=9 dzr 997764 > my.flac
dzr-api artist pink floyds | xargs dzr | mpv -
```

