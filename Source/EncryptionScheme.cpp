#include "EncryptionScheme.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <assert.h>

//--------------------------------------- EncryptionScheme ---------------------------------------

EncryptionScheme::EncryptionScheme()
{
}

/*virtual*/ EncryptionScheme::~EncryptionScheme()
{
}

//--------------------------------------- NoEncryptionScheme ---------------------------------------

NoEncryptionScheme::NoEncryptionScheme()
{
}

/*virtual*/ NoEncryptionScheme::~NoEncryptionScheme()
{
}

/*virtual*/ bool NoEncryptionScheme::Encrypt(const std::string& plainText, const std::string& password, std::vector<uint8_t>& cipherText)
{
	cipherText.reserve(plainText.length() + 1);
	::memcpy(cipherText.data(), plainText.c_str(), plainText.length() + 1);
	return true;
}

/*virtual*/ bool NoEncryptionScheme::Decrypt(const std::vector<uint8_t>& cipherText, const std::string& password, std::string& plainText)
{
	plainText = (const char*)cipherText.data();
	return true;
}

//--------------------------------------- AESEncryptionScheme ---------------------------------------

AESEncryptionScheme::AESEncryptionScheme()
{
}

/*virtual*/ AESEncryptionScheme::~AESEncryptionScheme()
{
}

/*virtual*/ bool AESEncryptionScheme::Encrypt(const std::string& plainText, const std::string& password, std::vector<uint8_t>& cipherText)
{
	bool success = false;
	int result = 0;
	EVP_CIPHER_CTX* context = nullptr;

	do
	{
		// This shouldn't happen, but if it does, we should bail, because we'll generate non-secure data.
		if (RAND_status() != 1)
			break;

		// We'll need salt to help generate our encryption key.
		uint8_t saltBuffer[16];
		uint32_t saltBufferSize = sizeof(saltBuffer);
		result = RAND_bytes(saltBuffer, saltBufferSize);
		if (result != 1)
			break;

		// Generate our encryption key as a function of the salt and the given password.
		uint8_t keyBuffer[32];
		uint32_t keyBufferSize = sizeof(keyBuffer);
		if (!this->MakeKey(password, saltBuffer, saltBufferSize, keyBuffer, keyBufferSize))
			break;

		uint8_t ivBuffer[16];
		uint32_t ivBufferSize = sizeof(ivBuffer);
		result = RAND_priv_bytes(ivBuffer, ivBufferSize);
		if (result != 1)
			break;

		context = EVP_CIPHER_CTX_new();
		if (!context)
			break;

		// The correct sizes of our buffers here are determined by the algorithm we're using.
		// The documentation is not clear at all how to verify the buffer sizes, though.
		result = EVP_EncryptInit_ex(context, EVP_aes_256_cbc(), nullptr, keyBuffer, ivBuffer);
		if (result != 1)
			break;

		const uint8_t* inBuffer = (const uint8_t*)plainText.c_str();
		int inBufferSize = (int)plainText.length() + 1;		// Add the null byte at the end.

		int blockSize = EVP_CIPHER_block_size(EVP_aes_256_cbc());
		int outBufferSize = saltBufferSize + ivBufferSize + inBufferSize + blockSize;
		cipherText.resize(outBufferSize);
		uint8_t* outBuffer = cipherText.data();
		::memset(outBuffer, 0, outBufferSize);
		
		::memcpy(outBuffer, saltBuffer, saltBufferSize);
		outBuffer += saltBufferSize;
		outBufferSize -= saltBufferSize;

		::memcpy(outBuffer, ivBuffer, ivBufferSize);
		outBuffer += ivBufferSize;
		outBufferSize -= ivBufferSize;
		
		int numBytesWritten = 0;
		result = EVP_EncryptUpdate(context, outBuffer, &numBytesWritten, inBuffer, inBufferSize);
		if (result != 1)
			break;

		assert(numBytesWritten <= outBufferSize);

		outBuffer += numBytesWritten;
		outBufferSize -= numBytesWritten;

		result = EVP_EncryptFinal_ex(context, outBuffer, &numBytesWritten);
		if (result != 1)
			break;

		assert(numBytesWritten <= outBufferSize);

		outBufferSize -= numBytesWritten;
		if (outBufferSize > 0)
			cipherText.resize(cipherText.size() - outBufferSize);

		success = true;
	} while (false);

	if (context)
	{
		EVP_CIPHER_CTX_free(context);
		context = nullptr;
	}

	return success;
}

/*virtual*/ bool AESEncryptionScheme::Decrypt(const std::vector<uint8_t>& cipherText, const std::string& password, std::string& plainText)
{
	bool success = false;
	int result = 0;
	EVP_CIPHER_CTX* context = nullptr;

	do
	{
		const uint8_t* inBuffer = (const uint8_t*)cipherText.data();
		uint32_t inBufferSize = (uint32_t)cipherText.size();

		uint8_t saltBuffer[16];
		uint32_t saltBufferSize = sizeof(saltBuffer);
		if (inBufferSize < saltBufferSize)
			break;

		::memcpy(saltBuffer, inBuffer, saltBufferSize);
		inBuffer += saltBufferSize;
		inBufferSize -= saltBufferSize;

		uint8_t ivBuffer[16];
		uint32_t ivBufferSize = sizeof(ivBuffer);
		if (inBufferSize < ivBufferSize)
			break;

		::memcpy(ivBuffer, inBuffer, ivBufferSize);
		inBuffer += ivBufferSize;
		inBufferSize -= ivBufferSize;

		// Generate the decryption key.  Note that this must be done in exactly the same way we did it during encryption.
		uint8_t keyBuffer[32];
		uint32_t keyBufferSize = sizeof(keyBuffer);
		if (!this->MakeKey(password, saltBuffer, saltBufferSize, keyBuffer, keyBufferSize))
			break;

		context = EVP_CIPHER_CTX_new();
		if (!context)
			break;

		// The correct sizes of our buffers here are determined by the algorithm we're using.
		result = EVP_DecryptInit_ex(context, EVP_aes_256_cbc(), nullptr, keyBuffer, ivBuffer);
		if (result != 1)
			break;

		// What remains in the input buffer at this point should be big-enough for our output.
		std::vector<uint8_t> memory;
		memory.resize(inBufferSize);
		::memset(memory.data(), 0, inBufferSize);
		uint8_t* outBuffer = memory.data();
		uint32_t outBufferSize = (uint32_t)memory.size();

		int numBytesWritten = 0;
		result = EVP_DecryptUpdate(context, outBuffer, &numBytesWritten, inBuffer, inBufferSize);
		if (result != 1)
			break;

		assert(numBytesWritten <= outBufferSize);

		outBuffer += numBytesWritten;
		outBufferSize -= numBytesWritten;

		result = EVP_DecryptFinal_ex(context, outBuffer, &numBytesWritten);
		if (result != 1)
			break;

		assert(numBytesWritten <= outBufferSize);

		// We added a null-terminating byte during encryption, so we should be able to just assign to string.
		plainText = (const char*)memory.data();

		success = true;
	} while (false);

	if (context)
	{
		EVP_CIPHER_CTX_free(context);
		context = nullptr;
	}

	return success;
}

bool AESEncryptionScheme::MakeKey(const std::string& password, const uint8_t* saltBuffer, uint32_t saltBufferSize, uint8_t* keyBuffer, uint32_t keyBufferSize)
{
	assert(keyBufferSize == 32);

	int result = PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), saltBuffer, saltBufferSize, 200000, EVP_sha256(), keyBufferSize, keyBuffer);
	if (result != 1)
		return false;

	return true;
}