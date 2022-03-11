//taken from project-zs-ng, thx

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

#include <paths.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>

#define SHA_SALT_LEN 4

#ifndef  __USE_SVID
struct passwd pwd;
struct passwd *fgetpwent(FILE * fp)
{
    char *data[10], tmp;
    int charcnt[10], fieldcnt = 0, varsize = 0;

    charcnt[0] = 0;

    while (fread(&tmp, 1, 1, fp) > 0) {
	charcnt[fieldcnt]++;

	if (varsize < charcnt[fieldcnt]) {
	    varsize += 20;
	    if (varsize == 20)
		data[fieldcnt] = malloc(varsize);
	    else
		data[fieldcnt] = realloc(data[fieldcnt], varsize);
	}

	if (tmp == '\n') {
	    data[fieldcnt][charcnt[fieldcnt] - 1] = 0;
	    break;
	} else if (tmp == ':') {
	    data[fieldcnt][charcnt[fieldcnt] - 1] = 0;
	    fieldcnt++;
	    charcnt[fieldcnt] = varsize = 0;
	} else {
	    data[fieldcnt][charcnt[fieldcnt] - 1] = tmp;
	}
    }

    if (fieldcnt == 6) {
	pwd.pw_name = malloc(charcnt[0]);
	pwd.pw_passwd = malloc(charcnt[1]);
	pwd.pw_gecos = malloc(charcnt[4]);
	strcpy(pwd.pw_name, data[0]);
	strcpy(pwd.pw_passwd, data[1]);
	strcpy(pwd.pw_gecos, data[4]);
    } else
	return NULL;

    return &pwd;
}
#endif

int pw_encrypt_new(const unsigned char *pwd, unsigned char *encryp, char *digest)
{
    unsigned char hexconvert[3];
    unsigned char *salt;
    int i;
    unsigned char md[SHA_DIGEST_LENGTH];
    int mdlen = SHA_DIGEST_LENGTH;

    unsigned char real_salt[SHA_SALT_LEN + 1];

    bzero(hexconvert, sizeof(hexconvert));

    salt = encryp;
    salt++;
    for (i = 0; i < SHA_SALT_LEN; i++) {
	hexconvert[0] = (*salt);
	salt++;
	hexconvert[1] = (*salt);
	salt++;
	real_salt[i] = strtol(hexconvert, NULL, 16);
    }

    PKCS5_PBKDF2_HMAC_SHA1(pwd, strlen(pwd), real_salt, SHA_SALT_LEN, 100,
			   mdlen, md);

    *digest = '$';
    digest++;
    for (i = 0; i < SHA_SALT_LEN; i++) {
	sprintf(digest, "%02x", real_salt[i]);
	digest += 2;
    }
    *digest = '$';
    digest++;
    for (i = 0; i < mdlen; i++) {
	sprintf(digest, "%02x", md[i]);
	digest += 2;
    }
    //fix the last /0 !!!
    *digest = '\0';

}

int main(int argc, char *argv[])
{
    FILE *fp;
    struct passwd *buf;

    char *crypted;
    char salt[2];

    if (argc != 4) {
	printf("Usage: %s <user> <pass> <passwdfile>\n", argv[0]);
	return 1;
    }
    if ((fp = fopen(argv[3], "r")) == NULL) {
	printf("Ooops, couldn\'t open your passwd file.\n");
	printf("Looks like you didnt specify a correct path.\n");
	return 1;
    }
    while ((buf = fgetpwent(fp)) != NULL) {
	if (strcmp(buf->pw_name, argv[1]))
	    continue;

	if (strlen(buf->pw_passwd) == 50) {
	    crypted = malloc(51);
	    if (!crypted) {
		printf
		    ("Ooops, couldn't allocate %d bytes of memory for hash.\n",
		     (SHA_DIGEST_LENGTH * 2 + 1));
		return 1;
	    }
	    pw_encrypt_new((unsigned char *) argv[2], buf->pw_passwd,
			   crypted);
	} else {
	    printf
		("Ooops, password is of invalid length! (not gl1 nor gl2).\n");
	    return 1;
	}
	
	//printf("%s %s\n",buf->pw_passwd, crypted);

	if (strcmp(buf->pw_passwd, crypted) == 0) {
	    printf("1\n");
	    return 1;
	} else {
	    printf("0\n");
	    return 0;
	}
    }
    printf("No such user in passwd file!\n");
    return 0;
}
