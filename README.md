<img width=100% height=200 src=.logo.svg>

# Usage

```
dzr [TRACKID...]
```

⚠️ For legal reasons, `dzr` does contain any encryption/decryption keys.

However they can easily be found by
[reversing](https://github.com/yne/dzr/wiki) the HTML5 player.
Then given to `dzr` via [environment variables](#environment-variables).

# Exemples

```sh
dzr 997764           # basic usage
dzr 997764 | mpv -   # piped streaming
dzr 997764 > my.mp3  # redirect namming
dzr 997764 997763    # multiple tracks
```

# API Examples

```sh
curl api.deezer.com/artist/27/top?limit=500| jq .data[].id
curl api.deezer.com/playlist/3631662942 | jq .tracks.data[].id
curl api... | jq ... | shuf | xargs dzr | mpv -
```

# dzr-db

`dzr` must be authenticated (via `DZR_SID`) in order to get full tracks URLs.
However, if the track is old enough, `dzr-db` can get the URL from it ID.
To do that, it request an anonymous external DataBase.

This allows you to use `dzr` account-less,
but probably wont work for recent tracks that haven't been added in the DB.

```sh
dzr-db 997764 | xargs dzr | mpv -
```

# Environment variables

  - `DZR_SID` = your_sid (optional) [HOWTO](https://github.com/yne/dzr/wiki)
  - `DZR_AES` = `jo..............` [HOWTO](https://github.com/yne/dzr/wiki)
  - `DZR_CBC` = `g4el............` [HOWTO](https://github.com/yne/dzr/wiki)
  - `DZR_FMT` = `0` (can be: 128Kb:`0`, 320Kb:`3`, AAC96:`8`, FLAC:`9`)

