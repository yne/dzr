//NIX build `gcc dzr.c -lssl -lcrypto`
//WIN build `tcc dzr.c libssl32.def libeay32.def ws2_32.def`
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock.h>
#else

#include <sys/socket.h>
#include <unistd.h>
#include <resolv.h>
#include <netdb.h>
#include <arpa/inet.h>

#define closesocket close
#define WSACleanup() ;
#define WSAStartup(A, B) ;
#endif

#include <openssl/ssl.h>
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/blowfish.h>

#define expect(cond, fmt...) if(!(cond))return fprintf(stderr,"expect(" #cond ") failed \n" fmt),-1;
#define HOST_WWW "www.deezer.com"
#define HOST_API "api.deezer.com"
#define HOST_CDN "e-cdn-proxy-0.deezer.com"
#define GW(method) "/ajax/gw-light.php?method=" method "&api_version=1.0&input=3&api_token="
#define PKCS5(total, current) (int)((total) - strlen(current))

#define USAGE "USAGE:\n"\
              "  dzr [TRACK...]\n"\
              "ENVIRONMENT:\n"\
              "  - DZR_SID=%s\n"\
              "  - DZR_AES=%s\n"\
              "  - DZR_CBC=%s\n"\
              "  - DZR_FMT=%s (opt) one of 128K:0, 320K:3, AAC96:8, FLAC:9\n"\
              "  - DEBUG=1: (opt)\n"\
              "EXEMPLES:\n"\
              "  sid=fr123456789 dzr 600629 # sid via local env\n"\
              "  dzr deezer.com/en/track/526893582 # url is ok\n"\
              "  dzr 572537042 572537052 # batch processing\n"\
              "  dzr 600629 > my.mp3 # manual output redirect\n"\
              "  dzr 600629 | mpv - # streaming (--cache-secs=30)\n"\
              "  curl api.dzr | jq ... | shuf | xargs dzr | mpv -\n"\
							"API EXAMPLES:\n"\
              "  curl api.deezer.com/artist/27/top?limit=500| jq .data[].id\n"\
              "  curl api.deezer.com/playlist/3631662942 | jq .tracks.data[].id\n"\
              ""
/* according to RE on the JS (bullshit) */
// MP3_MISC=0,MP3_128=1,MP3_320=3,MP3_256=5,MP3_192=7,AAC_96=8FLAC=9,MP3_64=10,MP3_32=11

#define md5(line) MD5((unsigned char *) (line), strlen(line), (unsigned char[16]) {})

char *find(char *buf, const char *pattern) {
	char *found = strstr(buf, pattern);
	if (!found)
		fprintf(stderr, "pattern '%s' not found\n", pattern);
	return found ? found + strlen(pattern) : "";
}

int debug(int color, int size, void *buf) {
	if (getenv("DEBUG"))fprintf(stderr, "\033[%i;1m%*.*s\033[0m", color, size, size, (char *) buf);
	return size;
}

int http_recv(SSL *ssl, void *data, int max) {
	return debug(31, SSL_read(ssl, data, max), data);
}

int http_send(SSL *ssl, void *data) {
	return debug(32, SSL_write(ssl, data, (int) strlen(data)), data);
}

char *toHex(unsigned char *bin, size_t count, char *hex, char *fmt) {
	for (size_t i = 0; i < count; i++)
		sprintf(hex + (2 * i), fmt, bin[i]);
	return hex;
}

int fetch(char *meth, char *path, char *params, char *host, char **fields, char **bodies, char *out, size_t outmax,
          int(*cb)(SSL*, size_t, void**), void**cb_args) {
	int len = 0, tcp = socket(AF_INET, SOCK_STREAM, 0);
	size_t clen = 0;
	expect(!connect(tcp, (const struct sockaddr *) &((struct sockaddr_in) {
	 .sin_family = AF_INET, .sin_port = htons(443), .sin_addr.s_addr = *(in_addr_t *) (gethostbyname(host)->h_addr)
	}), sizeof(struct sockaddr_in)));

	SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
	SSL *ssl = SSL_new(ctx);
	SSL_set_fd(ssl, tcp);
	expect(SSL_connect(ssl) != -1);
	for (int i = 0; bodies && bodies[i]; i++)
		len += strlen(bodies[i]);
	char bodylen[9];
	sprintf(bodylen, "%i", len);
	char *header[] = {meth, " ", path, params ? params : "", " HTTP/1.0\r\n", "Host: ", host, "\r\n", "Content-Length: ", bodylen, "\r\n", 0};
	for (int i = 0; header[i]; i++) http_send(ssl, header[i]);
	for (int i = 0; fields && fields[i]; i++) http_send(ssl, fields[i]);
	http_send(ssl, "\r\n");
	for (int i = 0; bodies && bodies[i]; i++) http_send(ssl, bodies[i]);
	for (len = 0; http_recv(ssl, out + len, 1) > 0 && !(len > 3 && out[len] == '\n' && !strncmp(out + len - 3, "\r\n\r\n", 4)); len++);
	out[len] = 0;
	if (strstr(out, "Content-Length:")) {
		clen = (int) strtol(find(out, "Content-Length:"), NULL, 0);
		outmax = outmax < clen ? outmax : clen;
	}
	if (cb) {
		cb(ssl, clen, cb_args);
	} else {
		size_t pos = 0;
		while (pos < outmax && http_recv(ssl, out + pos + len, 1) > 0)
			++pos;
		out[pos + len] = '\0';//TODO : rework
	}

	SSL_shutdown(ssl);
	SSL_free(ssl);
	closesocket(tcp);
	SSL_CTX_free(ctx);
	return tcp;
}

char *getTokenFromSID(char *sid, char token[32 + 1]) {
	char page[1024 * 32];
	fetch("GET", GW("deezer.getUserData"), "", HOST_WWW,
	      (char *[]) {"Cookie: sid=", sid, "\r\n", 0}, NULL,
	      page, sizeof(page), NULL, NULL);
	return memcpy(token, find(page, "checkForm\":\""), 32);
}
//fre84ab3c3a885631cdec4f9b874c0e67692b53e
int getTrackInfo(char *sid, char *tkn, char *trackid, char md5[32 + 1], char artist[64 + 1], char title[64 + 1]) {
	char page[1024 * 32];
	if (*md5) {
		return memcpy(title, md5, 64),0;
	}
	fetch("POST", GW("song.getListData"), tkn, HOST_WWW,
	      (char *[]) {"Cookie: sid=", sid, "\r\n", 0},
	      (char *[]) {"{\"sng_ids\":[", trackid, "]}", 0},
	      page, sizeof(page), NULL, NULL);
	memcpy(md5, find(page, "MD5_ORIGIN\":\""), 32);
	memcpy(artist, find(page, "SNG_TITLE\":\""), 64);
	memcpy(title, find(page, "ART_NAME\":\""), 64);
	return (int) strtol(find(page, "MEDIA_VERSION\":\""), NULL, 0);
}

char *getTrackUrl(char *md5, int version, char *trackid, char *format, char url[160 + 1], AES_KEY*aes_key) {
	char line[50] = {};
	snprintf(line, sizeof(line), "%.32s\xA4%s\xA4%s\xA4%i", md5, format, trackid, version);
	char *line_md5 = toHex(md5(line), 16, (char[32 + 48 + 1]) {}, "%02x");
	sprintf(line_md5 + 32, "\xA4%s\xA4%c%c%c", line, PKCS5(46, line), PKCS5(46, line), PKCS5(46, line));
	unsigned char pth[80];
	for (size_t i = 0; i < sizeof(pth); i += 16)
		AES_ecb_encrypt((unsigned char *) line_md5 + i, pth + i, aes_key, AES_ENCRYPT);
	return toHex(pth, sizeof(pth), url, "%02X");
}

BF_KEY *getTrackKey(char *trackid, const char*bf, BF_KEY *bf_key) {
	char *trackid_md5 = toHex(md5(trackid), 16, (char[32 + 1]) {}, "%02x");
	unsigned char xor[16];
	for (size_t i = 0; i < sizeof(xor); i++)
		xor[i] = (unsigned char) (bf[i] ^ trackid_md5[i] ^ trackid_md5[i + 16]);
	BF_set_key(bf_key, sizeof(xor), xor);
	return bf_key;
	//char dbg[128]={};toHex(xor, dbg, sizeof(xor), "%02x");fprintf(stderr, "decrypting with: %s\n", dbg);
}
int decryptTrack(SSL* ssl, size_t mp3_size, void**args) {
	BF_KEY *bf_key = args[0];
	FILE *fd = args[1];
	unsigned char chunk[2048];
	ssize_t total = 0, ret, nbchunk = 0;
	while ((ret = http_recv(ssl, chunk + (total % sizeof(chunk)), sizeof(chunk) - (total % sizeof(chunk)))) > 0) {
		if ((total += ret) % sizeof(chunk))
			continue;//not yet complet chunk, continue recving
		if (!(nbchunk++ % 3)) {
			BF_cbc_encrypt(chunk, chunk, sizeof(chunk), bf_key, (unsigned char[]) {0, 1, 2, 3, 4, 5, 6, 7}, BF_DECRYPT);
		}
		if (fwrite(chunk, sizeof(chunk), 1, fd) != 1)break;
		fprintf(stderr, "%c %zi\r", "/-\\|"[nbchunk / 512 % 4], total);
		if (total + sizeof(chunk) > mp3_size) { // incomplete last chunk
			for (ssize_t remain = mp3_size - total; remain > 0 && http_recv(ssl, chunk, 1) == 1; remain--)
				fwrite(chunk, 1, 1, fd);
			break;
		}
	}
	return fprintf(stderr, "\n");
}
int fetchTrack(BF_KEY *bf_key, char *path, char cdnId, FILE *fd) {
	char page[1024 * 32], cdn[24 + 1];
	memcpy(cdn, HOST_CDN, sizeof(cdn));
	cdn[12] = cdnId;
	fprintf(stderr, "  http://%s/mobile/1/%s\n", cdn, path);
	return fetch("GET", "/mobile/1/", path, cdn, NULL, NULL, page, sizeof(page), decryptTrack, (void*[]){bf_key, fd});
}

int main(int argc, char *argv[]) {
	argv++, argc--;
	char *sid = getenv("DZR_SID")?:getenv("sid")?:getenv("SID");
	char *bf = getenv("DZR_CBC"), *aes=getenv("DZR_AES"), *fmt = getenv("DZR_FMT") ?: "0";
	if (argc < 1 || !bf || !aes)
		return fprintf(stderr, USAGE, sid, aes, bf, fmt);
	WSAStartup(MAKEWORD(2, 2), &(WSADATA) {});
	SSL_library_init();

	AES_KEY aes_key;// Used for CDN URL Generation
	AES_set_encrypt_key((unsigned char *) aes, 128, &aes_key);

	char *token = getTokenFromSID(sid, (char[32 + 1]) {});
	//if (dbg)fprintf(stderr, "token=%s\n", token);

	for (char *track = *argv; argc; --argc, track = *++argv) {
		char md5[32 + 1] = {}, artist[64] = {}, title[64] = {};
		if (!strcmp(track,"http")) { // track URL : shift arg up to it the first digit
			for (; *track && !isdigit(*track); track++);
		} else if (strlen(track) >= 32 && track[32]==':') { // "MD5:*:id" format
			memcpy(md5,track,sizeof(md5)-1);
			track += sizeof(md5);
		}
		int version = getTrackInfo(sid, token, track, md5, artist, title);
		if(!*md5){continue;}/* private/blocked track */
		int artist_len = *artist ? (int) (strchr(artist, '"') - artist) : 0;
		int title_len = *title ? (int) (strchr(title, '"') - title) : 0;
		//if(dbg)fprintf(stderr, "version=%i md5=%s : %.*s - %.*s\n", version, md5, artist_len, artist, title_len, title);
		char *url = getTrackUrl(md5, version, track, fmt, (char[160 + 1]) {}, &aes_key);
		FILE *fd = stdout;
		if (isatty(STDOUT_FILENO)) { // TTY == no STDOUT piping => output to a pre-named file
			char mp3name[1024], *ext = fmt[0] == '9' ? "flac" : "mp3";
			sprintf(mp3name, "%.*s%s%.*s.%s", artist_len, artist, (*artist && *title) ? " - " : "", title_len, title, ext);
			fd = fopen(mp3name, "wb+");
		}
		fetchTrack(getTrackKey(track, bf, (BF_KEY[1]) {}), url, md5[0], fd);
		if (fd != stdout)fclose(fd);
	}
	WSACleanup();
	return 0;
}
