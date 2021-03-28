<img width=100% height=200 src=".github/.logo.svg">
<p align=center>Your favorite flows, straight from shell</p>

> ⚠️ For legal reasons, this project does not provide the [decyption key](https://github.com/yne/dzr/wiki)  

# Preview

[![](https://asciinema.org/a/NpET2MMpGN41QW2a0JOjFru0l.svg)](https://asciinema.org/a/NpET2MMpGN41QW2a0JOjFru0l)

# Requirement

- `apt install wget jq openssl`
- Set the [decryption key](https://github.com/yne/dzr/wiki) `export DZR_CBC=g*******0zvf9na1` in your `.bashrc`/`.zshrc`/`.profile`

# Usage Example (using curl + mpv)

```sh
# Note: DZR_CBC must be set for dzr-dec to work
./dzr-url 5404528 664107 | while read url id; do curl -s "$url" | ./dzr-dec $id | mpv - ; done
```

> You shall wrap the above line in a frontend `dzr` script (change mpv/curl if needed) so you would just have to type `dzr 5404528 664107` (see Preview)
