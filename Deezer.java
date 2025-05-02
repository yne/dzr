package f.f.freezer;

import android.util.Log;
import android.util.Pair;

import org.jaudiotagger.audio.AudioFile;
import org.jaudiotagger.audio.AudioFileIO;
import org.jaudiotagger.tag.FieldKey;
import org.jaudiotagger.tag.Tag;
import org.jaudiotagger.tag.TagOptionSingleton;
import org.jaudiotagger.tag.flac.FlacTag;
import org.jaudiotagger.tag.id3.ID3v23Tag;
import org.jaudiotagger.tag.id3.valuepair.ImageFormats;
import org.jaudiotagger.tag.images.Artwork;
import org.jaudiotagger.tag.images.ArtworkFactory;
import org.jaudiotagger.tag.reference.PictureTypes;
import org.json.JSONArray;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.RandomAccessFile;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.Arrays;
import java.util.Scanner;

import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;
import javax.net.ssl.HttpsURLConnection;

public class Deezer {

    DownloadLog logger;
    String token;
    String arl;
    String sid;
    String accessToken;
    boolean authorized = false;
    boolean authorizing = false;

    Deezer() {}

    //Initialize for logging
    void init(DownloadLog logger, String arl) {
        //Load native
        System.loadLibrary("decryptor-jni");

        this.logger = logger;
        this.arl = arl;
    }

    public native void decryptFile(String trackId, String inputFilename, String outputFilename);

    //Authorize GWLight API
    public void authorize() {
        if (!authorized || sid == null || token == null) {
            authorizing = true;
            try {
                callGWAPI("deezer.getUserData", "{}");
                authorized = true;
            } catch (Exception e) {
                logger.warn("Error authorizing to Deezer API! " + e.toString());
            }
        }
        //TV API
        if (accessToken == null) {
            try {
                authorizeTVAPI();
            } catch (Exception e) {
                this.accessToken = null;
                e.printStackTrace();
                logger.warn("Error authorizing TV API - FLAC might not be available! " + e.toString());
            }
        }

        authorizing = false;
    }

    //Make POST request
    private String POST(String _url, String data, String cookie) throws Exception {
        URL url = new URL(_url);
        HttpsURLConnection connection = (HttpsURLConnection) url.openConnection();
        connection.setConnectTimeout(20000);
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        connection.setRequestProperty("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.130 Safari/537.36");
        connection.setRequestProperty("Accept-Language", "*");
        connection.setRequestProperty("Content-Type", "application/json");
        connection.setRequestProperty("Accept", "*/*");
        connection.setRequestProperty("Content-Length", Integer.toString(data.getBytes(StandardCharsets.UTF_8).length));
        if (cookie != null) {
            connection.setRequestProperty("Cookie", cookie);
        }

        //Write body
        DataOutputStream wr = new DataOutputStream(connection.getOutputStream());
        wr.writeBytes(data);
        wr.close();
        //Get response
        String output = "";
        Scanner scanner = new Scanner(connection.getInputStream());
        while (scanner.hasNext()) {
            output += scanner.nextLine();
        }
        scanner.close();
        connection.disconnect();

        return output;
    }

    private String GET(String _url) throws Exception {
        URL url = new URL(_url);
        HttpsURLConnection connection = (HttpsURLConnection) url.openConnection();
        connection.setConnectTimeout(20000);
        connection.setRequestMethod("GET");
        connection.setRequestProperty("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.130 Safari/537.36");
        connection.setRequestProperty("Accept-Language", "*");
        connection.setRequestProperty("Accept", "*/*");
        //Get response
        String output = "";
        Scanner scanner = new Scanner(connection.getInputStream());
        while (scanner.hasNext()) {
            output += scanner.nextLine();
        }
        scanner.close();
        connection.disconnect();

        return output;
    }

    //Authorize TV API for generating URLs
    public void authorizeTVAPI() throws Exception {
        JSONObject json = new JSONObject(POST(
                "https://distribution-api.deezer.com/device/token",
                "{\"brand_name\":\"Hisense\",\"device_id\":\"7239e4071d8992c955ad\",\"model_name\":\"HE50A6109FUWTS\",\"country_code\":\"FRA\"}",
                null
        ));
        String deviceToken = json.getString("device_token");
        // Get unauthorized token
        json = new JSONObject(GET("https://connect.deezer.com/oauth/access_token.php?grant_type=client_credentials&client_id=447462&client_secret=a83bf7f38ad2f137e444727cfc3775cf&output=json"));
        String accessToken  = json.getString("access_token");
        // Get smart login code
        json = new JSONObject(POST(
                "https://connect.deezer.com/2.0/smartlogin/device?access_token=" + accessToken + "&device_token=" + deviceToken,
                "", null
        ));
        String smartLoginCode = json.getJSONObject("data").getString("smartLoginCode");
        // Get the fuck that is
        callGWAPI("deezer.associateSmartLoginCodeWithUser", "{\"SMARTLOGIN_CODE\": \"" + smartLoginCode + "\"}");
        // Get authorized access tonk
        json = new JSONObject(GET("https://connect.deezer.com/2.0/smartlogin/" + smartLoginCode + "?access_token=" + accessToken));
        accessToken = json.getJSONObject("data").getString("accessToken");
        this.accessToken = accessToken;
        Log.d("DDDD", "Authorized TV: " + this.accessToken);
    }

    public JSONObject callGWAPI(String method, String params) throws Exception {
        //Get token
        if (token == null) {
            token = "null";
            callGWAPI("deezer.getUserData", "{}");
        }

        String data = POST(
                "https://www.deezer.com/ajax/gw-light.php?method=" + method + "&input=3&api_version=1.0&api_token=" + token,
                params,
                "arl=" + arl + "; sid=" + sid
        );

        //Parse JSON
        JSONObject out = new JSONObject(data);

        //Save token
        if ((token == null || token.equals("null")) && method.equals("deezer.getUserData")) {
            token = out.getJSONObject("results").getString("checkForm");
            sid = out.getJSONObject("results").getString("SESSION_ID");
        }

        return out;
    }


    //api.deezer.com/$method/$param
    public static JSONObject callPublicAPI(String method, String param) throws Exception {
        URL url = new URL("https://api.deezer.com/" + method + "/" + param);
        HttpsURLConnection connection = (HttpsURLConnection)url.openConnection();
        connection.setRequestMethod("GET");
        connection.setConnectTimeout(20000);
        connection.connect();

        //Get string data
        String data = "";
        Scanner scanner = new Scanner(url.openStream());
        while (scanner.hasNext()) {
            data += scanner.nextLine();
        }

        //Close
        try {
            connection.disconnect();
            scanner.close();
        } catch (Exception e) {}

        //Parse JSON
        JSONObject out = new JSONObject(data);
        return out;
    }

    //Generate track download URL
    public static String generateTrackUrl(String trackId, String md5origin, String mediaVersion, int quality) {
        try {
            int magic = 164;

            ByteArrayOutputStream step1 = new ByteArrayOutputStream();
            step1.write(md5origin.getBytes());
            step1.write(magic);
            step1.write(Integer.toString(quality).getBytes());
            step1.write(magic);
            step1.write(trackId.getBytes());
            step1.write(magic);
            step1.write(mediaVersion.getBytes());
            //Get MD5
            MessageDigest md5 = MessageDigest.getInstance("MD5");
            md5.update(step1.toByteArray());
            byte[] digest = md5.digest();
            String md5hex = bytesToHex(digest).toLowerCase();

            //Step 2
            ByteArrayOutputStream step2 = new ByteArrayOutputStream();
            step2.write(md5hex.getBytes());
            step2.write(magic);
            step2.write(step1.toByteArray());
            step2.write(magic);

            //Pad step2 with dots, to get correct length
            while(step2.size()%16 > 0) step2.write(46);

            //Prepare AES encryption
            Cipher cipher = Cipher.getInstance("AES/ECB/NoPadding");
            SecretKeySpec key = new SecretKeySpec("jo6aey6haid2Teih".getBytes(), "AES");
            cipher.init(Cipher.ENCRYPT_MODE, key);
            //Encrypt
            StringBuilder step3 = new StringBuilder();
            for (int i=0; i<step2.size()/16; i++) {
                byte[] b = Arrays.copyOfRange(step2.toByteArray(), i*16, (i+1)*16);
                step3.append(bytesToHex(cipher.doFinal(b)).toLowerCase());
            }
            //Join to URL
            String url = "https://e-cdns-proxy-" + md5origin.charAt(0) + ".dzcdn.net/mobile/1/" + step3.toString();
            return url;

        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    // Returns URL and wether encrypted
    public Pair<String, Boolean> getTrackUrl(String trackId, String md5origin, String mediaVersion, int quality) {
        // TV API URL Gen
        if (this.accessToken != null && quality == 9) {
            try {
                URL url = new URL("https://api.deezer.com/platform/gcast/track/" + trackId + "/streamUrls");
                HttpsURLConnection connection = (HttpsURLConnection) url.openConnection();
                connection.setConnectTimeout(20000);
                connection.setRequestMethod("GET");
                connection.setRequestProperty("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.130 Safari/537.36");
                connection.setRequestProperty("Accept-Language", "*");
                connection.setRequestProperty("Accept", "*/*");
                connection.setRequestProperty("Authorization", "Bearer " + this.accessToken);
                //Get response
                String output = "";
                Scanner scanner = new Scanner(connection.getInputStream());
                while (scanner.hasNext()) {
                    output += scanner.nextLine();
                }
                scanner.close();
                connection.disconnect();

                JSONObject json = new JSONObject(output).getJSONObject("data").getJSONObject("attributes");
                return new Pair<String, Boolean>(json.getString("url_flac"), false);
            } catch (Exception e) {
                e.printStackTrace();
                logger.warn("Failed generating ATV URL!");
            }
        }
        // Normal url gen
        return new Pair<String, Boolean>(Deezer.generateTrackUrl(trackId, md5origin, mediaVersion, quality), true);
    }

    public static String bytesToHex(byte[] bytes) {
        final char[] HEX_ARRAY = "0123456789ABCDEF".toCharArray();
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars);
    }

    public static String sanitize(String input) {
        return input.replaceAll("[\\\\/?*:%<>|\"]", "").replace("$", "\\$");
    }

    public static String generateFilename(String original, JSONObject publicTrack, JSONObject publicAlbum, int newQuality) throws Exception {
        original = original.replaceAll("%title%", sanitize(publicTrack.getString("title")));
        original = original.replaceAll("%album%", sanitize(publicTrack.getJSONObject("album").getString("title")));
        original = original.replaceAll("%artist%", sanitize(publicTrack.getJSONObject("artist").getString("name")));
        // Album might not be available
        try {
            original = original.replaceAll("%albumArtist%", sanitize(publicAlbum.getJSONObject("artist").getString("name")));
        } catch (Exception e) {
            original = original.replaceAll("%albumArtist%", sanitize(publicTrack.getJSONObject("artist").getString("name")));
        }

        //Artists
        String artists = "";
        String feats = "";
        for (int i=0; i<publicTrack.getJSONArray("contributors").length(); i++) {
            String artist = publicTrack.getJSONArray("contributors").getJSONObject(i).getString("name");
            if (!artists.contains(artist))
                artists += ", " + artist;
            if (i > 0 && !artists.contains(artist) && !feats.contains(artist))
                feats += ", " + artist;
        }
        original = original.replaceAll("%artists%", sanitize(artists).substring(2));
        if (feats.length() >= 2)
            original = original.replaceAll("%feats%", sanitize(feats).substring(2));
        //Track number
        int trackNumber = publicTrack.getInt("track_position");
        original = original.replaceAll("%trackNumber%", Integer.toString(trackNumber));
        original = original.replaceAll("%0trackNumber%", String.format("%02d", trackNumber));
        //Year
        original = original.replaceAll("%year%", publicTrack.getString("release_date").substring(0, 4));
        original = original.replaceAll("%date%", publicTrack.getString("release_date"));

        //Remove leading dots
        original = original.replaceAll("/\\.+", "/");

        if (newQuality == 9) return original + ".flac";
        return original + ".mp3";
    }

    //Deezer patched something so getting metadata of user uploaded MP3s is not working anymore
    public static String generateUserUploadedMP3Filename(String original, String title) throws Exception {
        String[] ignored = {"%feats%", "%trackNumber%", "%0trackNumber%", "%year%", "%date%", "%album%", "%artist%", "%artists%", "%albumArtist%"};
        for (String i : ignored) {
            original = original.replaceAll(i, "");
        }

        original = original.replace("%title%", sanitize(title));
        return original;
    }

    //Tag track with data from API
    public void tagTrack(String path, JSONObject publicTrack, JSONObject publicAlbum, String cover, JSONObject lyricsData, JSONObject privateJson, DownloadService.DownloadSettings settings) throws Exception {
        TagOptionSingleton.getInstance().setAndroid(true);
        //Load file
        AudioFile f = AudioFileIO.read(new File(path));
        boolean isFlac = true;
        if (f.getAudioHeader().getFormat().contains("MPEG")) {
            f.setTag(new ID3v23Tag());
            isFlac = false;
        }
        Tag tag = f.getTag();

        if (settings.tags.title) tag.setField(FieldKey.TITLE, publicTrack.getString("title"));
        if (settings.tags.album) tag.setField(FieldKey.ALBUM, publicTrack.getJSONObject("album").getString("title"));
        //Artist
        String artists = "";
        for (int i=0; i<publicTrack.getJSONArray("contributors").length(); i++) {
            String artist = publicTrack.getJSONArray("contributors").getJSONObject(i).getString("name");
            if (!artists.contains(artist))
                artists += settings.artistSeparator + artist;
        }
        boolean albumAvailable = !publicAlbum.has("error");
        if (settings.tags.artist) tag.addField(FieldKey.ARTIST, artists.substring(settings.artistSeparator.length()));
        if (settings.tags.track) tag.setField(FieldKey.TRACK, String.format("%02d", publicTrack.getInt("track_position")));
        if (settings.tags.disc) tag.setField(FieldKey.DISC_NO, Integer.toString(publicTrack.getInt("disk_number")));
        if (settings.tags.albumArtist && albumAvailable) tag.setField(FieldKey.ALBUM_ARTIST, publicAlbum.getJSONObject("artist").getString("name"));
        if (settings.tags.date) tag.setField(FieldKey.YEAR, publicTrack.getString("release_date").substring(0, 4));
        if (settings.tags.label && albumAvailable) tag.setField(FieldKey.RECORD_LABEL, publicAlbum.getString("label"));
        if (settings.tags.isrc) tag.setField(FieldKey.ISRC, publicTrack.getString("isrc"));
        if (settings.tags.upc && albumAvailable) tag.setField(FieldKey.BARCODE, publicAlbum.getString("upc"));
        if (settings.tags.trackTotal && albumAvailable) tag.setField(FieldKey.TRACK_TOTAL, Integer.toString(publicAlbum.getInt("nb_tracks")));

        //BPM
        if (publicTrack.has("bpm") && (int)publicTrack.getDouble("bpm") > 0)
            if (settings.tags.bpm) tag.setField(FieldKey.BPM, Integer.toString((int)publicTrack.getDouble("bpm")));

        //Unsynced lyrics
        if (lyricsData != null && settings.tags.lyrics) {
            try {
                String lyrics = lyricsData.getString("LYRICS_TEXT");
                tag.setField(FieldKey.LYRICS, lyrics);
            } catch (Exception e) {
                Log.w("WARN", "Error adding unsynced lyrics!");
            }
        }

        //Genres
        String genres = "";
        if (albumAvailable) {
            for (int i=0; i<publicAlbum.getJSONObject("genres").getJSONArray("data").length(); i++) {
                String genre = publicAlbum.getJSONObject("genres").getJSONArray("data").getJSONObject(0).getString("name");
                if (!genres.contains(genre)) {
                    genres += ", " + genre;
                }
            }
            if (genres.length() > 2 && settings.tags.genre)
                tag.setField(FieldKey.GENRE, genres.substring(2));
        }

        //Additional tags from private api
        if (settings.tags.contributors)
        try {
            if (privateJson != null && privateJson.has("SNG_CONTRIBUTORS")) {
                JSONObject contrib = privateJson.getJSONObject("SNG_CONTRIBUTORS");
                //Composer
                if (contrib.has("composer")) {
                    JSONArray composers = contrib.getJSONArray("composer");
                    String composer = "";
                    for (int i = 0; i < composers.length(); i++)
                        composer += settings.artistSeparator + composers.getString(i);
                    if (composer.length() > 2)
                        tag.setField(FieldKey.COMPOSER, composer.substring(settings.artistSeparator.length()));
                }
                //Engineer
                if (contrib.has("engineer")) {
                    JSONArray engineers = contrib.getJSONArray("engineer");
                    String engineer = "";
                    for (int i = 0; i < engineers.length(); i++)
                        engineer += settings.artistSeparator + engineers.getString(i);
                    if (engineer.length() > 2)
                        tag.setField(FieldKey.ENGINEER, engineer.substring(settings.artistSeparator.length()));
                }
                //Mixer
                if (contrib.has("mixer")) {
                    JSONArray mixers = contrib.getJSONArray("mixer");
                    String mixer = "";
                    for (int i = 0; i < mixers.length(); i++)
                        mixer += settings.artistSeparator + mixers.getString(i);
                    if (mixer.length() > 2)
                        tag.setField(FieldKey.MIXER, mixer.substring(settings.artistSeparator.length()));
                }
                //Producer
                if (contrib.has("producer")) {
                    JSONArray producers = contrib.getJSONArray("producer");
                    String producer = "";
                    for (int i = 0; i < producers.length(); i++)
                        producer += settings.artistSeparator + producers.getString(i);
                    if (producer.length() > 2)
                        tag.setField(FieldKey.MIXER, producer.substring(settings.artistSeparator.length()));
                }

                //FLAC Only
                if (isFlac) {
                    //Author
                    if (contrib.has("author")) {
                        JSONArray authors = contrib.getJSONArray("author");
                        String author = "";
                        for (int i = 0; i < authors.length(); i++)
                            author += settings.artistSeparator + authors.getString(i);
                        if (author.length() > 2)
                            ((FlacTag) tag).setField("AUTHOR", author.substring(settings.artistSeparator.length()));
                    }
                    //Writer
                    if (contrib.has("writer")) {
                        JSONArray writers = contrib.getJSONArray("writer");
                        String writer = "";
                        for (int i = 0; i < writers.length(); i++)
                            writer += settings.artistSeparator + writers.getString(i);
                        if (writer.length() > 2)
                            ((FlacTag) tag).setField("WRITER", writer.substring(settings.artistSeparator.length()));
                    }
                }
            }
        } catch (Exception e) {
            logger.warn("Error writing contributors data: " + e.toString());
        }

        File coverFile = new File(cover);
        boolean addCover = (coverFile.exists() && coverFile.length() > 0);

        if (isFlac) {
            //FLAC Specific tags
            if (settings.tags.date) ((FlacTag)tag).setField("DATE", publicTrack.getString("release_date"));
            //Cover
            if (addCover && settings.tags.albumArt) {
                RandomAccessFile cf = new RandomAccessFile(coverFile, "r");
                byte[] coverData = new byte[(int) cf.length()];
                cf.read(coverData);
                tag.setField(((FlacTag)tag).createArtworkField(
                    coverData,
                    PictureTypes.DEFAULT_ID,
                    ImageFormats.MIME_TYPE_JPEG,
                    "cover",
                    settings.albumArtResolution,
                    settings.albumArtResolution,
                    24,
                    0
                ));
            }
        } else {
            if (addCover && settings.tags.albumArt) {
                Artwork art = ArtworkFactory.createArtworkFromFile(coverFile);
                //Artwork art = Artwork.createArtworkFromFile(coverFile);
                tag.addField(art);
            }
        }

        //Save
        AudioFileIO.write(f);
    }

    //Create JSON file, privateJsonData = `song.getLyrics`
    public static String generateLRC(JSONObject privateJsonData, JSONObject publicTrack) throws Exception {
        String output = "";

        //Create metadata
        String title = publicTrack.getString("title");
        String album = publicTrack.getJSONObject("album").getString("title");
        String artists = "";
        for (int i=0; i<publicTrack.getJSONArray("contributors").length(); i++) {
            artists += ", " + publicTrack.getJSONArray("contributors").getJSONObject(i).getString("name");
        }
        //Write metadata
        output += "[ar:" + artists.substring(2) + "]\r\n[al:" + album + "]\r\n[ti:" + title + "]\r\n";

        //Get lyrics
        int counter = 0;
        JSONArray syncLyrics = privateJsonData.getJSONArray("LYRICS_SYNC_JSON");
        for (int i=0; i<syncLyrics.length(); i++) {
            JSONObject lyric = syncLyrics.getJSONObject(i);
            if (lyric.has("lrc_timestamp") && lyric.has("line")) {
                output += lyric.getString("lrc_timestamp") + lyric.getString("line") + "\r\n";
                counter += 1;
            }
        }

        if (counter == 0) throw new Exception("Empty Lyrics!");
        return output;
    }

    //Track decryption key
    static byte[] getKey(String id) {
        String secret = "g4el58wc0zvf9na1";
        try {
            MessageDigest md5 = MessageDigest.getInstance("MD5");
            md5.update(id.getBytes());
            byte[] md5id = md5.digest();
            String idmd5 = bytesToHex(md5id).toLowerCase();
            String key = "";
            for(int i=0; i<16; i++) {
                int s0 = idmd5.charAt(i);
                int s1 = idmd5.charAt(i+16);
                int s2 = secret.charAt(i);
                key += (char)(s0^s1^s2);
            }
            return key.getBytes();
        } catch (Exception e) {
            Log.e("E", e.toString());
            return new byte[0];
        }
    }

    //Decrypt 2048b of data
    static byte[] decryptChunk(byte[] key, byte[] data) {
        try {
            byte[] IV = {00, 01, 02, 03, 04, 05, 06, 07};
            SecretKeySpec Skey = new SecretKeySpec(key, "Blowfish");
            Cipher cipher = Cipher.getInstance("Blowfish/CBC/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE, Skey, new javax.crypto.spec.IvParameterSpec(IV));
            return cipher.doFinal(data);
        }catch (Exception e) {
            Log.e("D", e.toString());
            return new byte[0];
        }
    }

    static class QualityInfo {
        int quality;
        String md5origin;
        String mediaVersion;
        String trackId;
        int initialQuality;
        DownloadLog logger;
        boolean encrypted;

        QualityInfo(int quality, String trackId, String md5origin, String mediaVersion, DownloadLog logger) {
            this.quality = quality;
            this.initialQuality = quality;
            this.trackId = trackId;
            this.mediaVersion = mediaVersion;
            this.md5origin = md5origin;
            this.logger = logger;
        }

        String fallback(Deezer deezer) {
            //Quality fallback
            try {
                String url = qualityFallback(deezer);
                //No quality
                if (quality == -1)
                    throw new Exception("No quality to fallback to!");

                //Success
                return url;
            } catch (Exception e) {
                logger.warn("Quality fallback failed! ID: " + trackId + " " + e.toString());
                quality = initialQuality;
            }

            //Track ID Fallback
            JSONObject privateJson = null;
            try {
                //Fetch meta
                JSONObject privateRaw = deezer.callGWAPI("deezer.pageTrack", "{\"sng_id\": \"" + trackId + "\"}");
                privateJson = privateRaw.getJSONObject("results").getJSONObject("DATA");
                if (privateJson.has("FALLBACK")) {
                    //Fetch new track
                    String fallbackId = privateJson.getJSONObject("FALLBACK").getString("SNG_ID");
                    if (!fallbackId.equals(trackId)) {
                        JSONObject newPrivate = deezer.callGWAPI("song.getListData", "{\"sng_ids\": [" + fallbackId + "]}");
                        JSONObject trackData = newPrivate.getJSONObject("results").getJSONArray("data").getJSONObject(0);
                        trackId = trackData.getString("SNG_ID");
                        md5origin = trackData.getString("MD5_ORIGIN");
                        mediaVersion = trackData.getString("MEDIA_VERSION");
                        return fallback(deezer);
                    }
                }
            } catch (Exception e) {
                logger.error("ID fallback failed! ID: " + trackId + " " + e.toString());
            }

            //ISRC Fallback
            try {
                JSONObject newTrackJson = Deezer.callPublicAPI("track", "isrc:" + privateJson.getString("ISRC"));
                //Same track check
                if (newTrackJson.getInt("id") == Integer.parseInt(trackId)) throw new Exception("No more to ISRC fallback!");
                //Get private data
                privateJson = deezer.callGWAPI("song.getListData", "{\"sng_ids\": [" + newTrackJson.getInt("id") + "]}");
                JSONObject trackData = privateJson.getJSONObject("results").getJSONArray("data").getJSONObject(0);
                trackId = trackData.getString("SNG_ID");
                md5origin = trackData.getString("MD5_ORIGIN");
                mediaVersion = trackData.getString("MEDIA_VERSION");
                return fallback(deezer);
            } catch (Exception e) {
                logger.error("ISRC Fallback failed, track unavailable! ID: " + trackId + " " + e.toString());
            }

            return null;
        }

        private String qualityFallback(Deezer deezer) throws Exception {
            Pair<String,Boolean> urlGen = deezer.getTrackUrl(trackId, md5origin, mediaVersion, quality);
            this.encrypted = urlGen.second;

            //Create HEAD requests to check if exists
            URL url = new URL(urlGen.first);
            HttpsURLConnection connection = (HttpsURLConnection) url.openConnection();
            connection.setRequestMethod("HEAD");
            connection.setRequestProperty("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.130 Safari/537.36");
            connection.setRequestProperty("Accept-Language", "*");
            connection.setRequestProperty("Accept", "*/*");
            int rc = connection.getResponseCode();
            //Track not available
            if (rc > 400) {
                logger.warn("Quality fallback, response code: " + Integer.toString(rc) + ", current: " + Integer.toString(quality));
                //-1 if no quality available
                if (quality == 1) {
                    quality = -1;
                    return null;
                }
                if (quality == 3) quality = 1;
                if (quality == 9) quality = 3;
                return qualityFallback(deezer);
            }
            return urlGen.first;
        }

    }

}
