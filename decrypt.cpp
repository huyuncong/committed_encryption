#include <emp-tool/emp-tool.h>
#include <gcrypt.h>

// function taken from the Metal project
void aesgcm_decrypt(unsigned char *input, unsigned char *output, const int input_len, int *output_len, uint8_t *key, bool *valid){
	uint8_t iv[16];
	memcpy(iv, input, 16);

	gcry_cipher_hd_t handle;
	gcry_cipher_open(&handle, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_GCM, 0);
	gcry_cipher_setkey(handle, key, 16);
	gcry_cipher_setctr(handle, iv, 16);

	int blocks = (input_len - 32) / 16;

	gcry_cipher_decrypt(handle, output, 16 * blocks, &input[16], 16 * blocks);

	uint8_t tag[16];
	memcpy(tag, &input[16 + 16 * blocks], 16);

	gcry_error_t checktag_error = gcry_cipher_checktag(handle, tag, 16);
	if(gcry_err_code(checktag_error) == GPG_ERR_CHECKSUM){
		*valid = false;
	}else{
		*valid = true;
	}

	*output_len = 16 * blocks;
	gcry_cipher_close(handle);
}

void load_private_key(bn_t bn, int party){
	char filename[100];
	sprintf(filename, "sk_%d", party);

	FILE *fp;
	fp = fopen(filename, "rb");
	int bn_size;
	if(fp == NULL){
		printf("cannot open the private key file.\n");
		exit(1);
	}
	fread(&bn_size, sizeof(int), 1, fp);

	uint64_t bn_buffer[4];
	memset(bn_buffer, 0, 4 * sizeof(uint64_t));
	fread(bn_buffer, 4, sizeof(uint64_t), fp);
	
	bn_read_raw(bn, bn_buffer, bn_size);
}

int main(int argc, char **argv){
	if(argc != 4){
		printf("format: decrypt <partyID> <filename_in> <filename_out>\n");
		return 0;
	}

	emp::block s;
        emp::block k;
	
	bn_t sk;

	emp::initialize_relic();

	emp::bn_newl(sk);

	int c_1_part_1_size;
        int c_2_part_1_size;
        int c_3_part_1_size;
	int message_encrypted_len;
	int message_original_len;

	FILE *fp;
	fp = fopen(argv[2], "rb");
	if(fp == NULL){
		printf("cannot open the input file.\n");
		return 0;
	}

	fread(&c_1_part_1_size, sizeof(int), 1, fp);
	fread(&c_2_part_1_size, sizeof(int), 1, fp);
	fread(&c_3_part_1_size, sizeof(int), 1, fp);
	fread(&message_encrypted_len, sizeof(int), 1, fp);
	fread(&message_original_len, sizeof(int), 1, fp);

	unsigned char *message_encrypted = (unsigned char*) calloc(message_encrypted_len, 1);
	unsigned char *message_decrypted = (unsigned char*) calloc(message_encrypted_len, 1);

	uint8_t c_1_part_1_buffer[EB_SIZE];
        uint8_t c_2_part_1_buffer[EB_SIZE];
        uint8_t c_3_part_1_buffer[EB_SIZE];

        memset(c_1_part_1_buffer, 0, sizeof(uint8_t) * EB_SIZE);
        memset(c_2_part_1_buffer, 0, sizeof(uint8_t) * EB_SIZE);
        memset(c_3_part_1_buffer, 0, sizeof(uint8_t) * EB_SIZE);

	fread(c_1_part_1_buffer, sizeof(uint8_t), EB_SIZE, fp);
	fread(c_2_part_1_buffer, sizeof(uint8_t), EB_SIZE, fp);
	fread(c_3_part_1_buffer, sizeof(uint8_t), EB_SIZE, fp);

	eb_t c_1_part_1;
        eb_t c_1_part_2_num;
        emp::block c_1_part_2;

        eb_t c_2_part_1;
        eb_t c_2_part_2_num;
        emp::block c_2_part_2;

        eb_t c_3_part_1;
        eb_t c_3_part_2_num;
        emp::block c_3_part_2;

        emp::eb_newl(c_1_part_1, c_1_part_2_num, c_2_part_1, c_2_part_2_num, c_3_part_1, c_3_part_2_num);	

	eb_read_bin(c_1_part_1, c_1_part_1_buffer, c_1_part_1_size);
	eb_read_bin(c_2_part_1, c_2_part_1_buffer, c_2_part_1_size);
	eb_read_bin(c_3_part_1, c_3_part_1_buffer, c_3_part_1_size);

	fread(&c_1_part_2, sizeof(emp::block), 1, fp);
	fread(&c_2_part_2, sizeof(emp::block), 1, fp);
	fread(&c_3_part_2, sizeof(emp::block), 1, fp);

	fread(message_encrypted, message_encrypted_len, 1, fp);
	fclose(fp);

	if(argv[1][0] == '1'){
		load_private_key(sk, 1);
		emp::eb_mul_norm(c_1_part_2_num, c_1_part_1, sk);

		emp::block tmp;
		tmp = emp::KDF(c_1_part_2_num);

		s = emp::xorBlocks(tmp, c_1_part_2);
		
		emp::PRG prg_s(&s);
		prg_s.random_block(&k);
	}

	if(argv[1][0] == '2'){
		load_private_key(sk, 2);
		emp::eb_mul_norm(c_2_part_2_num, c_2_part_1, sk);

		emp::block tmp;
                tmp = emp::KDF(c_2_part_2_num);

                k = emp::xorBlocks(tmp, c_2_part_2);
	}

	if(argv[1][0] == '3'){
                load_private_key(sk, 3);
                emp::eb_mul_norm(c_3_part_2_num, c_3_part_1, sk);
                
                emp::block tmp;
                tmp = emp::KDF(c_3_part_2_num);
                
                k = emp::xorBlocks(tmp, c_3_part_2);
        }
	
	bool valid;
	int output_len;

	aesgcm_decrypt(message_encrypted, message_decrypted, message_encrypted_len, &output_len, (uint8_t *)&k, &valid);

	if(valid == false){
		printf("decrypted result is invalid.\n");
		exit(0);
	}

	fp = fopen(argv[3], "wb");
	if(fp == NULL){
		printf("cannot open the output file.\n");
		exit(1);
	}
	fwrite(message_decrypted, message_original_len, 1, fp);
	fclose(fp);

	emp::bn_freel(sk);
	free(message_decrypted);
	free(message_encrypted);
	emp::eb_freel(c_1_part_1, c_1_part_2_num, c_2_part_1, c_2_part_2_num, c_3_part_1, c_3_part_2_num);

	return 0;
}
