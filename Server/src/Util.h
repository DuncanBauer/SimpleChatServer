#pragma once

#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>

long long getSecondsSinceEpoch()
{
    // get the current time
    const auto now = std::chrono::system_clock::now();

    // transform the time into a duration since the epoch
    const auto epoch = now.time_since_epoch();

    // cast the duration into seconds
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    // return the number of seconds
    return seconds.count();
}

// Function to generate a random salt
std::string generateSalt(size_t length) {
    CryptoPP::AutoSeededRandomPool rng;
    std::vector<CryptoPP::byte> salt(length);
    rng.GenerateBlock(salt.data(), length);

    std::string encodedSalt;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(encodedSalt));
    encoder.Put(salt.data(), length);
    encoder.MessageEnd();
    return encodedSalt;
}

// Function to hash a password with a given salt
std::string hashPassword(const std::string& password, const std::string& salt) {
    std::string saltedPassword = salt + password;
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

    CryptoPP::SHA256 hash;
    hash.CalculateDigest(digest, (CryptoPP::byte*)saltedPassword.data(), saltedPassword.size());

    std::string hashedPassword;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(hashedPassword));
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();
    return hashedPassword;
}