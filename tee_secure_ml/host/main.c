/*
 * Copyright (c) 2017, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* TA API: UUID and command IDs */
#include <tee_secure_ml_ta.h>

/* TEE resources */
struct test_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};

void prepare_tee_session(struct test_ctx *ctx)
{
	TEEC_UUID uuid = TA_TEE_SECURE_ML_UUID;
	uint32_t origin;
	TEEC_Result res;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx->ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/* Open a session with the TA */
	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, origin);
}

void terminate_tee_session(struct test_ctx *ctx)
{
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

TEEC_Result ca_secure_ml_inference(struct test_ctx *ctx, char *id,
			char *infer, size_t infer_len, char *result, size_t result_len)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	op.params[1].tmpref.buffer = infer;
	op.params[1].tmpref.size = infer_len;

	op.params[2].tmpref.buffer = result;
	op.params[2].tmpref.size = result_len;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_TEE_SECURE_ML_CMD_INFERENCE,
				 &op, &origin);
	switch (res) {
	case TEEC_SUCCESS:
	case TEEC_ERROR_SHORT_BUFFER:
	case TEEC_ERROR_ITEM_NOT_FOUND:
		break;
	default:
		printf("Command READ_RAW failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

TEEC_Result ca_secure_ml_weight_init(struct test_ctx *ctx, char *id,
			char *data, size_t data_len)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	op.params[1].tmpref.buffer = data;
	op.params[1].tmpref.size = data_len;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_TEE_SECURE_ML_CMD_INIT,
				 &op, &origin);
	if (res != TEEC_SUCCESS)
		printf("Command WRITE_RAW failed: 0x%x / %u\n", res, origin);

	switch (res) {
	case TEEC_SUCCESS:
		break;
	default:
		printf("Command WRITE_RAW failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

TEEC_Result ca_secure_ml_delete(struct test_ctx *ctx, char *id)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_TEE_SECURE_ML_CMD_DELETE,
				 &op, &origin);

	switch (res) {
	case TEEC_SUCCESS:
	case TEEC_ERROR_ITEM_NOT_FOUND:
		break;
	default:
		printf("Command DELETE failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

void char_to_float(char *c_array, float *f_array, size_t float_array_size){

	float f;
  	char sBuf[4];

	for (int i = 0; i < float_array_size; i++){
		sBuf[0] = c_array[i*4];
		sBuf[1] = c_array[i*4+1];
		sBuf[2] = c_array[i*4+2];
		sBuf[3] = c_array[i*4+3];
		f = *(float*)(&sBuf);	
		f_array[i] = f;	
	}
}

void float_to_char(float *f_array, char *c_array, size_t float_array_size){
    char* temp;

    for (int i = 0; i < float_array_size; i++){
        temp = (char*)(f_array + i);
        c_array[i*4] = temp[0];
        c_array[i*4+1] = temp[1];
        c_array[i*4+2] = temp[2];
        c_array[i*4+3] = temp[3];
	}

}

void print_float_array(float *f_array, size_t size){
	for (int i = 0; i < size; i++){
		printf("%g ", f_array[i]);
	}
	printf("\n");
}

#define LOGISTIC_DIM 4
#define KNN_CLASS 10
#define NN_DIM 4
#define NN_CLASS 3

int main(int argc, char *argv[])
{
	struct test_ctx ctx;
	TEEC_Result res;

	printf("Prepare session with the TA\n");
	prepare_tee_session(&ctx);

	/*
	 * Test for logistic regression,
	 * init weight, inference with it, delete weight.
	 */
	printf("\nTest TEE logistic regression \n");

	printf("- Create and load LR weight in the TA secure storage\n");

	char obj1_id[] = "1";		/* string identification for the algorithm */

	float log_weight[LOGISTIC_DIM] = {0.2, 0.5, -0.25, -0.4};
	char log_weight_byte[4 * LOGISTIC_DIM];
	float_to_char(log_weight, log_weight_byte, LOGISTIC_DIM);
	printf("The weight for LR is: ");
	print_float_array(log_weight, LOGISTIC_DIM);

	float log_infer[LOGISTIC_DIM] = {0.5, 0.2, 0.4, 0.25};
	char log_infer_byte[4 * LOGISTIC_DIM];
	float_to_char(log_infer, log_infer_byte, LOGISTIC_DIM);

	float log_result[1];
	char log_result_byte[4];

	res = ca_secure_ml_weight_init(&ctx, obj1_id,
				  log_weight_byte, sizeof(log_weight_byte));
	if (res != TEEC_SUCCESS)
		errx(1, "Failed to init weight for logistic regression in the secure storage\n");

	printf("- Find LR weight in TEE and do inference\n");
	printf("The inference instance for LR is: ");
	print_float_array(log_infer, LOGISTIC_DIM);

	res = ca_secure_ml_inference(&ctx, obj1_id, log_infer_byte, sizeof(log_infer_byte), 
				 log_result_byte, sizeof(log_result_byte));
	if (res != TEEC_SUCCESS)
		errx(1, "Failed to read an object from the secure storage\n");
	char_to_float(log_result_byte, log_result, 1);
	printf("Logistic Regression Result: %g\n", log_result[0]);

	printf("- Delete LR weight in TEE\n");

	res = ca_secure_ml_delete(&ctx, obj1_id);
	if (res != TEEC_SUCCESS)
		errx(1, "Failed to delete the weight: 0x%x\n", res);

	
	/*
	 * Test for KNN
	 * init weight, inference with it, delete weight.
	 */
	printf("\nTest TEE KNN.\n");

	printf("- Create and load weight in the TA secure storage\n");

	char obj2_id[] = "2";		/* string identification for the algorithm */

	float knn_weight[KNN_CLASS] = {-0.5, -0.4, -0.3, -0.2, -0.1, 0, 0.1, 0.2, 0.3, 0.4};
	char knn_weight_byte[4 * KNN_CLASS];
	float_to_char(knn_weight, knn_weight_byte, KNN_CLASS);
	printf("The weight for KNN is: ");
	print_float_array(knn_weight, KNN_CLASS);

	float knn_infer[1] = {0.17};
	char knn_infer_byte[4];
	float_to_char(knn_infer, knn_infer_byte, 1);

	float knn_result[1];
	char knn_result_byte[4];

	res = ca_secure_ml_weight_init(&ctx, obj2_id,
				  knn_weight_byte, sizeof(knn_weight_byte));
	if (res != TEEC_SUCCESS)
		errx(1, "Failed to init weight for knn in the secure storage\n");

	printf("- Find KNN weight in TEE and do inference\n");
	printf("The inference instance for KNN is: ");
	print_float_array(knn_infer, 1);

	res = ca_secure_ml_inference(&ctx, obj2_id, knn_infer_byte, sizeof(knn_infer_byte), 
				 knn_result_byte, sizeof(knn_result_byte));
	if (res != TEEC_SUCCESS)
		errx(1, "Failed to read an object from the secure storage\n");
	char_to_float(knn_result_byte, knn_result, 1);
	printf("KNN Result: %g\n", knn_result[0]);

	printf("- Delete KNN weight in TEE\n");

	res = ca_secure_ml_delete(&ctx, obj2_id);
	if (res != TEEC_SUCCESS)
		errx(1, "Failed to delete the weight: 0x%x\n", res);



	/*
	 * Test for Neural Network
	 * init weight, inference with it, delete weight.
	 */
	printf("\nTest TEE neural network.\n");

	printf("- Create and load neural network weight in the TA secure storage\n");

	char obj3_id[] = "3";		/* string identification for the algorithm */

	float nn_weight[ (NN_DIM +1) * NN_CLASS] = {-0.5, -0.4, 0.3, -0.2, 0.5,
												 0, 0.1, 0.2, -0.3, 0.5,
												 0.2, -0.2, -0.3, 0.4, 0.5};
	char nn_weight_byte[4 * (NN_DIM +1) * NN_CLASS];
	float_to_char(nn_weight, nn_weight_byte, (NN_DIM +1) * NN_CLASS);
	printf("The weight for neural network is: \n");
	print_float_array(nn_weight, (NN_DIM +1) * NN_CLASS);

	float nn_infer[NN_DIM] = {0.17, 0.13, -0.1, -0.4};
	char nn_infer_byte[4 * NN_DIM];
	float_to_char(nn_infer, nn_infer_byte, NN_DIM);

	float nn_result[NN_CLASS];
	char nn_result_byte[4 * NN_CLASS];

	res = ca_secure_ml_weight_init(&ctx, obj3_id,
				  nn_weight_byte, sizeof(nn_weight_byte));
	if (res != TEEC_SUCCESS)
		errx(1, "Failed to init weight for knn in the secure storage\n");

	printf("- Find neural network weight in TEE and do inference\n");
	printf("The inference instance for neural network is: ");
	print_float_array(nn_infer, NN_DIM);

	res = ca_secure_ml_inference(&ctx, obj3_id, nn_infer_byte, sizeof(nn_infer_byte), 
				 nn_result_byte, sizeof(nn_result_byte));
	if (res != TEEC_SUCCESS)
		errx(1, "Failed to read an object from the secure storage\n");
	char_to_float(nn_result_byte, nn_result, NN_CLASS);
	printf("Neural Network Result: %g, %g, %g\n", nn_result[0], nn_result[1], nn_result[2]);

	printf("- Delete neural network weight in TEE\n");

	res = ca_secure_ml_delete(&ctx, obj3_id);
	if (res != TEEC_SUCCESS)
		errx(1, "Failed to delete the weight: 0x%x\n", res);

	printf("\nWe're done, close and release TEE resources\n");
	terminate_tee_session(&ctx);
	return 0;
}
