# Committed encryption
A new multi-recipient encryption scheme for the CS294-144 Blockchain project, in which the first party can check whether the ciphertext is generated by a honest encryption.

Technical details are [here](https://github.com/weikengchen/committed_encryption/raw/master/doc/main.pdf).

## Components

- `keygen`: generate the public/private key pairs for all three parties.
- `encrypt`: generate a ciphertext for the three parties.
- `decrypt`: for any one of the three parties, decrypt the ciphertext.
- `check`: the first party checks whether the three parties can decrypt into the same result.
- `encrypt_malicious`: an example of malicious encryption where the randomness is not generated correctly.

## Example usage

We can use `doc/main.tex` as the test file:

```
cmake . -DUSE_RANDOM_DEVICE=1
cd bin
cp ../doc/main.tex ./
```

We can use the honest encryption to encrypt the file, and use any one of the three parties to decrypt.

```
./encrypt main.tex main_encrypted
./decrypt 1 main_encrypted main_decrypted.tex
./decrypt 2 main_encrypted main_decrypted.tex
./decrypt 3 main_encrypted main_decrypted.tex
```

We can let the first party check whether the ciphertext is generated honestly.

```
./check main_encrypted
```

A malicious encryption will be detected.

```
./encrypt_malicious main.tex main_encrypted
./check main_encrypted
```
which outputs:

```
the party 3's first part is not from a honest encryption.
```


## Dependency and Acknowledgement
This project has a depedency of emp-toolkit/emp-tool. We use the key derivation function (KDF) with CPU-instruction SHA1. emp-tool also provides good interfaces for 128-bit block data structures.

We use relic toolkit for ECC. The code on using relic toolkit is from emp-toolkit/emp-ot.

We use AES-GCM based on libgcrypt. 

We use the Findgcrypt script for cmake from libssh.
