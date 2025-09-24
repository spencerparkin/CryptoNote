#pragma once

#include <string>
#include <vector>

/**
 * This will be the base class for any support encryption scheme.
 */
class EncryptionScheme
{
public:
	EncryptionScheme();
	virtual ~EncryptionScheme();

	virtual bool Encrypt(const std::string& plainText, const std::string& password, std::vector<uint8_t>&  cipherText) = 0;
	virtual bool Decrypt(const std::vector<uint8_t>& cipherText, const std::string& password, std::string& plainText) = 0;
};

/**
 * For debug purposes, here we'll just pass the plain text through unchanged.
 */
class NoEncryptionScheme : public EncryptionScheme
{
public:
	NoEncryptionScheme();
	virtual ~NoEncryptionScheme();

	virtual bool Encrypt(const std::string& plainText, const std::string& password, std::vector<uint8_t>& cipherText) override;
	virtual bool Decrypt(const std::vector<uint8_t>& cipherText, const std::string& password, std::string& plainText) override;
};

/**
 * Here we're going to use OpenSSL's AES-CBC symmetric key encryption algorithm.
 */
class AESEncryptionScheme : public EncryptionScheme
{
public:
	AESEncryptionScheme();
	virtual ~AESEncryptionScheme();

	virtual bool Encrypt(const std::string& plainText, const std::string& password, std::vector<uint8_t>& cipherText) override;
	virtual bool Decrypt(const std::vector<uint8_t>& cipherText, const std::string& password, std::string& plainText) override;

private:
	
	bool MakeKey(const std::string& password, const uint8_t* saltBuffer, uint32_t saltBufferSize, uint8_t* keyBuffer, uint32_t keyBufferSize);
};