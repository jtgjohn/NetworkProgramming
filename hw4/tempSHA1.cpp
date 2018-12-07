#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>

int main() {

	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	char input[] = "12345";
	unsigned char id[EVP_MAX_MD_SIZE];
	unsigned int md_len;

	//Create a context
	mdctx = EVP_MD_CTX_create();

	//Inititialize our digest to use the md TYPE object given by EVP_sha1
	//Need to do this everytime we want to hash something
	EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);

	//Add to the hash
	EVP_DigestUpdate(mdctx, input, strlen(input));

	//Finalize the hash, store in md_value
	EVP_DigestFinal_ex(mdctx, id, &md_len);

	//Free the context
	EVP_MD_CTX_destroy(mdctx);

	printf("%s\n", id);

	/*
	const unsigned char str[] = "Test String";
	unsigned char hash[8];//SHA_DIGEST_LENGTH]; //20

	SHA1(str, sizeof(str) - 1, hash);

	printf("%s", hash);
	*/
}