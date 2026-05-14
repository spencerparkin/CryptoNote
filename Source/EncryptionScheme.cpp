#include "EncryptionScheme.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <wx/log.h>
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

//--------------------------------------- OpenSSL_AES_EncryptionScheme ---------------------------------------

OpenSSL_AES_EncryptionScheme::OpenSSL_AES_EncryptionScheme()
{
}

/*virtual*/ OpenSSL_AES_EncryptionScheme::~OpenSSL_AES_EncryptionScheme()
{
}

/*virtual*/ bool OpenSSL_AES_EncryptionScheme::Encrypt(const std::string& plainText, const std::string& password, std::vector<uint8_t>& cipherText)
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

/*virtual*/ bool OpenSSL_AES_EncryptionScheme::Decrypt(const std::vector<uint8_t>& cipherText, const std::string& password, std::string& plainText)
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

bool OpenSSL_AES_EncryptionScheme::MakeKey(const std::string& password, const uint8_t* saltBuffer, uint32_t saltBufferSize, uint8_t* keyBuffer, uint32_t keyBufferSize)
{
	assert(keyBufferSize == 32);

	int result = PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), saltBuffer, saltBufferSize, 200000, EVP_sha256(), keyBufferSize, keyBuffer);
	if (result != 1)
		return false;

	return true;
}

//--------------------------------------- BCrypt_AES_EncryptionScheme ---------------------------------------

BCrypt_AES_EncryptionScheme::BCrypt_AES_EncryptionScheme()
{
}

/*virtual*/ BCrypt_AES_EncryptionScheme::~BCrypt_AES_EncryptionScheme()
{
}

/*virtual*/ bool BCrypt_AES_EncryptionScheme::Encrypt(const std::string& plainText, const std::string& password, std::vector<uint8_t>& cipherText)
{
	NTSTATUS status = 0;
	BCRYPT_KEY_HANDLE hKey = nullptr;
	BCRYPT_ALG_HANDLE hAesAlg = nullptr;

	status = BCryptOpenAlgorithmProvider(&hAesAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
	if (!BCRYPT_SUCCESS(status))
	{
		wxLogError("Failed to get AES algorithm provider.");
		return false;
	}

	status = BCryptSetProperty(hAesAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
	if (!BCRYPT_SUCCESS(status))
		return false;

	std::vector<BYTE> keyObject;
	if (!this->MakeKey(hAesAlg, hKey, keyObject, password))
		return false;

	// Make random initialization vector.
	BYTE iv[16];
	BCryptGenRandom(nullptr, iv, sizeof(iv), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

	// Copy IV now since it gets modified during the encryption process.
	BYTE originalIV[16];
	for (int i = 0; i < 16; i++)
		originalIV[i] = iv[i];

	// How big does the cipher text need to be?
	DWORD cipherTextSize = 0;
	status = BCryptEncrypt(hKey, (PUCHAR)plainText.data(), (ULONG)plainText.size(), nullptr, iv, sizeof(iv), nullptr, 0, &cipherTextSize, BCRYPT_BLOCK_PADDING);
	if (!BCRYPT_SUCCESS(status))
	{
		wxLogError("Failed to determine cipher text size.");
		return false;
	}

	cipherText.resize(cipherTextSize);

	// Encrypt!
	status = BCryptEncrypt(hKey, (PUCHAR)plainText.data(), (ULONG)plainText.size(), nullptr, iv, sizeof(iv), cipherText.data(), cipherTextSize, &cipherTextSize, BCRYPT_BLOCK_PADDING);
	if (!BCRYPT_SUCCESS(status))
	{
		wxLogError("Encryption failed!");
		return false;
	}

	// Tack this onto the cipher text since we'll need it later for decryption.
	for (int i = 0; i < sizeof(iv); i++)
		cipherText.push_back(originalIV[i]);

	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAesAlg, 0);

	return true;
}

/*virtual*/ bool BCrypt_AES_EncryptionScheme::Decrypt(const std::vector<uint8_t>& cipherText, const std::string& password, std::string& plainText)
{
	NTSTATUS status = 0;
	BCRYPT_KEY_HANDLE hKey = nullptr;
	BCRYPT_ALG_HANDLE hAesAlg = nullptr;

	status = BCryptOpenAlgorithmProvider(&hAesAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
	if (!BCRYPT_SUCCESS(status))
	{
		wxLogError("Failed to get AES algorithm provider.");
		return false;
	}

	BCryptSetProperty(hAesAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);

	std::vector<BYTE> keyObject;
	if (!this->MakeKey(hAesAlg, hKey, keyObject, password))
		return false;

	// Grab the IV we tacked onto the end of the cipher text.
	BYTE iv[16];
	for (int i = 0; i < 16; i++)
	{
		int j = int(cipherText.size()) - 16 + i;
		iv[i] = cipherText[j];
	}

	// How big is the plain text?
	DWORD decryptedSize = 0;
	status = BCryptDecrypt(hKey, (PUCHAR)cipherText.data(), cipherText.size() - 16, nullptr, iv, sizeof(iv), nullptr, 0, &decryptedSize, BCRYPT_BLOCK_PADDING);
	if (!BCRYPT_SUCCESS(status))
	{
		wxLogError("Could not determine plain text size of encrypted data.");
		return false;
	}

	// Decrypt!
	std::vector<BYTE> decrypted(decryptedSize);
	status = BCryptDecrypt(hKey, (PUCHAR)cipherText.data(), cipherText.size() - 16, nullptr, iv, sizeof(iv), decrypted.data(), decryptedSize, &decryptedSize, BCRYPT_BLOCK_PADDING);
	if (!BCRYPT_SUCCESS(status))
	{
		wxLogError("Failed to decrypt!");
		return false;
	}

	plainText.reserve(decryptedSize);
	plainText = "";
	for (int i = 0; i < decryptedSize; i++)
		plainText.push_back(decrypted[i]);

	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAesAlg, 0);

	return true;
}

bool BCrypt_AES_EncryptionScheme::MakeKey(BCRYPT_ALG_HANDLE hAesAlg, BCRYPT_KEY_HANDLE& hKey, std::vector<BYTE>& keyObject, const std::string& password)
{
	hKey = nullptr;

	NTSTATUS status = 0;
	BCRYPT_ALG_HANDLE hHashAlg = nullptr;
	BCRYPT_HASH_HANDLE hHash = nullptr;

	DWORD hashObjectSize = 0;
	DWORD dataSize = 0;
	DWORD hashSize = 0;

	status = BCryptOpenAlgorithmProvider(&hHashAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
	if (!BCRYPT_SUCCESS(status))
	{
		wxLogError("Failed to get SHA256 algorithm.");
		return false;
	}

	BCryptGetProperty(hHashAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashObjectSize, sizeof(DWORD), &dataSize, 0);
	BCryptGetProperty(hHashAlg, BCRYPT_HASH_LENGTH, (PUCHAR)&hashSize, sizeof(DWORD), &dataSize, 0);

	std::vector<BYTE> hashObject(hashObjectSize);
	std::vector<BYTE> key(hashSize);

	BCryptCreateHash(hHashAlg, &hHash, hashObject.data(), hashObjectSize, nullptr, 0, 0);

	BCryptHashData(hHash, (PUCHAR)password.c_str(), (ULONG)password.length(), 0);
	BCryptFinishHash(hHash, key.data(), hashSize, 0);

	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hHashAlg, 0);

	// Make a symmetric key.
	DWORD keyObjectSize = 0;
	DWORD cbData = 0;
	BCryptGetProperty(hAesAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&keyObjectSize, sizeof(DWORD), &cbData, 0);
	keyObject.resize(keyObjectSize);
	status = BCryptGenerateSymmetricKey(hAesAlg, &hKey, keyObject.data(), keyObjectSize, key.data(), (ULONG)key.size(), 0);
	if (!BCRYPT_SUCCESS(status))
	{
		wxLogError("Failed to make symmetric key.");
		return false;
	}

	return true;
}